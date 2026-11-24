# SPDX-License-Identifier: LGPL-2.1-or-later
"""Headless native verification; FreeCADCmd exit status alone is insufficient."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--freecad', help='Path to FreeCADCmd/freecadcmd')
    args = parser.parse_args()
    binary = args.freecad or shutil.which('FreeCADCmd') or shutil.which('freecadcmd')
    if binary is None:
        parser.error('Pass --freecad /path/to/FreeCADCmd')
    output = ROOT / '.build'
    output.mkdir(exist_ok=True)
    report_path = output / 'native-report.json'
    report_path.unlink(missing_ok=True)
    with tempfile.TemporaryDirectory(prefix='transaction-guard-') as temp:
        env = dict(os.environ, QT_QPA_PLATFORM='offscreen', TRANSACTION_GUARD_TEST_OUTPUT=temp)
        with (output / 'native.log').open('w') as log:
            result = subprocess.run([binary, str(ROOT/'tests/native_suite.py')],
                env=env, cwd=ROOT, stdin=subprocess.DEVNULL, stdout=log, stderr=subprocess.STDOUT, timeout=120)
        native_report = Path(temp) / 'native-report.json'
        if not native_report.is_file():
            raise SystemExit('No native report; inspect .build/native.log')
        report = json.loads(native_report.read_text())
        if result.returncode == 0 and report.get('passed'):
            native_report.unlink()
            env['TRANSACTION_GUARD_CASE'] = 'abort_failure'
            with (output/'abort-failure.log').open('w') as log:
                fault_result = subprocess.run([binary, str(ROOT/'tests/native_suite.py')],
                    env=env, cwd=ROOT, stdin=subprocess.DEVNULL, stdout=log, stderr=subprocess.STDOUT, timeout=120)
            if not native_report.is_file():
                raise SystemExit('Missing abort-failure report; inspect .build/abort-failure.log')
            fault = json.loads(native_report.read_text())
            report['abort_failure'] = fault
            report['passed'] = fault_result.returncode == 0 and fault.get('passed', False)
            report['checks'].extend(fault.get('checks', []))
        report['source_sha256'] = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in sorted(list((ROOT/'src').rglob('*.py')) + list((ROOT/'tests').glob('*.py')) + [Path(__file__)])}
        report_path.write_text(json.dumps(report, indent=2)+'\n')
        if result.returncode or not report.get('passed'):
            raise SystemExit('Native regression failed; inspect .build/native-report.json and native.log')
    print('PASS: %d native regression groups; .build/native-report.json' % len(report['checks']))


if __name__ == '__main__':
    main()
