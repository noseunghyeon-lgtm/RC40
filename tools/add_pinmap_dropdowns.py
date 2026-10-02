#!/usr/bin/env python3
"""Add the 'Use' column and multi-port dropdowns to RC40_Pinmap.xlsx.

  * H column 'Use' (Signal section): dropdown ON / OFF, default ON
      ON  -> pin generated in HwInp / HwOutp
      OFF -> pin generated in the 'Unused' subsystem
  * C column 'Type' (multi-port pins only): dropdown with the functions the
    pin supports, derived from its Description (e.g. A22 'Digital, Voltage,
    Current, Resistance' -> Frequency, DigitalSignal, AnalogSignal,
    CurrentSignal, ResistanceMeasurementSignal).

Standard library only (no openpyxl). Safe to run again: existing 'Use' values
are kept and the dropdowns are rebuilt.

Usage:  python add_pinmap_dropdowns.py [path/to/RC40_Pinmap.xlsx]

Keep RULES in sync with rc40_pin_functions.m.
"""
import re
import shutil
import sys
import zipfile
from xml.sax.saxutils import escape

RULES = [
    ('digital', 'DigitalSignal'),
    ('voltage', 'AnalogSignal'),
    ('analog/', 'AnalogSignal'),
    ('current', 'CurrentSignal'),
    ('resistance', 'ResistanceMeasurementSignal'),
    ('frequency', 'Frequency'),
    ('dsm1/dst1', 'Frequency'),
    ('sent', 'SENTSignal'),
    ('analog output', 'PWMSignal'),
]
SHEET = 'xl/worksheets/sheet1.xml'
SST = 'xl/sharedStrings.xml'


def pin_functions(desc, cur):
    d = desc.lower()
    types = [cur.strip()]
    multi = (',' in d) or ('/' in d and 'dsm1/dst1' not in d and not d.startswith('pull'))
    if 'output' in d and 'analog output' not in d:   # power/switching output text, not a function list
        multi = False
    if not multi:
        return types, False
    for key, typ in RULES:
        if key in d and typ.lower() not in [t.lower() for t in types]:
            types.append(typ)
    return types, len(types) > 1


def shared_strings(z):
    xml = z.read(SST).decode('utf-8')
    out = []
    for si in re.findall(r'<si>(.*?)</si>', xml, re.S):
        out.append(''.join(re.findall(r'<t[^>]*>(.*?)</t>', si, re.S)))
    return out


def cell_text(row_xml, col, sst):
    m = re.search(r'<c r="%s\d+"([^>]*?)(?:/>|>(.*?)</c>)' % col, row_xml, re.S)
    if not m or not m.group(2):
        return ''
    attrs, body = m.group(1), m.group(2)
    if 't="s"' in attrs:
        v = re.search(r'<v>(\d+)</v>', body)
        return sst[int(v.group(1))] if v else ''
    if 't="inlineStr"' in attrs:
        return ''.join(re.findall(r'<t[^>]*>(.*?)</t>', body, re.S))
    v = re.search(r'<v>(.*?)</v>', body)
    return v.group(1) if v else ''


def cell_style(row_xml, col):
    m = re.search(r'<c r="%s\d+" s="(\d+)"' % col, row_xml)
    return m.group(1) if m else None


def inline_cell(ref, text, style):
    s = ' s="%s"' % style if style else ''
    return '<c r="%s"%s t="inlineStr"><is><t>%s</t></is></c>' % (ref, s, escape(text))


def ranges(rows):
    """[3,4,5,9] -> 'X3:X5 X9' style groups (column added by caller)."""
    rows = sorted(rows)
    groups, start, prev = [], rows[0], rows[0]
    for r in rows[1:]:
        if r == prev + 1:
            prev = r
            continue
        groups.append((start, prev))
        start = prev = r
    groups.append((start, prev))
    return groups


def main(path):
    with zipfile.ZipFile(path) as z:
        sst = shared_strings(z)
        xml = z.read(SHEET).decode('utf-8')
        items = {n: z.read(n) for n in z.namelist()}

    row_re = re.compile(r'<row r="(\d+)"[^>]*>.*?</row>|<row r="(\d+)"[^>]*/>', re.S)
    section, use_rows, type_lists, new_rows = None, [], {}, {}
    for m in row_re.finditer(xml):
        rxml = m.group(0)
        r = int(m.group(1) or m.group(2))
        b, c = cell_text(rxml, 'B', sst), cell_text(rxml, 'C', sst)
        if b in ('Signal', 'Power', 'Communication') and not c:
            section = b
            continue
        if section != 'Signal' or not b:
            continue
        if b == 'Name' and c == 'Type':                       # header row
            if not cell_text(rxml, 'H', sst):
                new_rows[r] = (rxml, inline_cell('H%d' % r, 'Use', cell_style(rxml, 'G')))
            continue
        use_rows.append(r)
        if not re.search(r'<c r="H%d"' % r, rxml):           # keep existing ON/OFF
            new_rows[r] = (rxml, inline_cell('H%d' % r, 'ON', cell_style(rxml, 'D')))
        types, multi = pin_functions(cell_text(rxml, 'E', sst), c)
        if multi:
            type_lists.setdefault(','.join(types), []).append(r)

    for r, (rxml, cell) in new_rows.items():
        upd = rxml.replace('</row>', cell + '</row>')
        upd = re.sub(r'spans="(\d+):\d+"', r'spans="\1:8"', upd, count=1)
        xml = xml.replace(rxml, upd, 1)

    xml = re.sub(r'<dimension ref="([A-Z]+\d+):[A-Z]+(\d+)"/>', r'<dimension ref="\1:H\2"/>', xml)
    if '<col min="8"' not in xml:
        xml = xml.replace('</cols>', '<col min="8" max="8" width="9" customWidth="1"/></cols>')
    xml = xml.replace('<mergeCell ref="B1:G1"/>', '<mergeCell ref="B1:H1"/>')

    dvs = ['<dataValidation type="list" allowBlank="1" showInputMessage="1" showErrorMessage="1" '
           'errorTitle="Use" error="ON 또는 OFF를 선택하세요." promptTitle="Use" '
           'prompt="ON: HwInp/HwOutp에 생성, OFF: Unused에 생성" sqref="%s">'
           '<formula1>"ON,OFF"</formula1></dataValidation>'
           % ' '.join('H%d:H%d' % g if g[0] != g[1] else 'H%d' % g[0] for g in ranges(use_rows))]
    for lst, rows in type_lists.items():
        sq = ' '.join('C%d:C%d' % g if g[0] != g[1] else 'C%d' % g[0] for g in ranges(rows))
        dvs.append('<dataValidation type="list" allowBlank="1" showInputMessage="1" showErrorMessage="1" '
                   'errorTitle="Type" error="이 핀이 지원하는 기능 중에서 선택하세요." '
                   'promptTitle="Multi-port" prompt="핀 기능을 선택하세요 (easyConfig와 일치시킬 것)" '
                   'sqref="%s"><formula1>&quot;%s&quot;</formula1></dataValidation>' % (sq, lst))
    block = '<dataValidations count="%d">%s</dataValidations>' % (len(dvs), ''.join(dvs))
    xml = re.sub(r'<dataValidations.*?</dataValidations>', '', xml, flags=re.S)
    # OOXML order: ... mergeCells, phoneticPr, conditionalFormatting, dataValidations,
    # hyperlinks, printOptions, pageMargins ...  -> insert before the first of these
    for tag in ('<hyperlinks', '<printOptions', '<pageMargins', '<pageSetup'):
        if tag in xml:
            xml = xml.replace(tag, block + tag, 1)
            break

    shutil.copyfile(path, path + '.bak')
    items[SHEET] = xml.encode('utf-8')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as z:
        for name, data in items.items():
            z.writestr(name, data)

    print('Use column rows : %d' % len(use_rows))
    print('Multi-port rows : %d' % sum(len(v) for v in type_lists.values()))
    for lst, rows in type_lists.items():
        print('  %-70s %s' % (lst, ', '.join('C%d' % r for r in rows)))
    print('Backup          : %s.bak' % path)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else 'RC40_Pinmap.xlsx')
