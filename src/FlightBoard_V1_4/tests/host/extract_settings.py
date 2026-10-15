"""Extract production settings declarations/functions for host-only tests.

No replacement implementation: the compiled text comes directly from the .ino.
Secrets and board/network functions are never opened or included.
"""
from pathlib import Path
import argparse
import re

parser = argparse.ArgumentParser()
parser.add_argument('project', nargs='?', default='.')
parser.add_argument('--output', type=Path)
args = parser.parse_args()
source = (Path(args.project) / 'FlightBoard_V1_4.ino').read_text()
struct = source[source.index('struct BoardSettings {'):source.index('\nWebServer webServer(')]
functions = source[source.index('bool validApp('):source.index('\nvoid resetRadar()')]
brightness_globals = '\n'.join(re.search(r'^' + declaration + r'[^\n]*;', source, re.M).group(0)
    for declaration in ('int savedBrightness ', 'int savedNightBrightness ', 'uint32_t brightnessChangedMs ', 'bool brightnessSaveError '))
out = args.output or Path(__file__).with_name('settings_under_test.inc')
out.write_text(struct + '\nPreferences storage;\nFlightBoardFavorites favorites(storage);\nBoardSettings settings;\nString selectedHex;\n' + brightness_globals + '\n' + functions)
filters = source[source.index('bool matchesAviationFilters('):source.index('\nvoid collectRadar()')]
selection = source[source.index('int activeAircraftIndex()'):source.index('\n// Main-loop rendering only.')]
out.with_name('filters_under_test.inc').write_text(
    'AircraftInfo radarPlanes[MAX_AIRCRAFT];\nint radarCount = 0;\n' + filters + '\n' + selection)
out.with_name('zcs_handlers_under_test.inc').write_text(
    source[source.index('void changeApp()'):source.index('\nvoid startWebServer()')])
def production_function(signature):
    start = source.index(signature + ' {')
    return source[start:source.index('\n}', start) + 2]
out.with_name('brightness_under_test.inc').write_text('\n\n'.join(production_function(signature)
    for signature in ('void sendSettings()', 'bool readJsonBody(JsonDocument& doc)', 'void changeBrightness()')))
print('Extracted BoardSettings and actual settings functions from', args.project)
