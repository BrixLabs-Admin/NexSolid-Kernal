# SPDX-License-Identifier: LGPL-2.1-or-later
"""Executed by FreeCADCmd; creates only owned, disposable documents."""
import json
import os
import sys
import threading
from pathlib import Path

import FreeCAD as App
import Part

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'src'))
from freecad_transaction_guard import Guard, OwnershipError, RecoveryRequired

OUT = Path(os.environ['TRANSACTION_GUARD_TEST_OUTPUT'])
OUT.mkdir(parents=True, exist_ok=True)
checks = []


def check(value, message):
    if not value:
        raise AssertionError(message)


def fixture():
    doc = App.newDocument('TransactionGuardFixture')
    doc.UndoMode = 1
    body = doc.addObject('PartDesign::Body', 'Body')
    base = body.newObject('PartDesign::Feature', 'Base')
    base.Shape = Part.makeBox(40, 30, 10)
    doc.recompute()
    return doc, body, base


def blend(body, base, kind):
    # Correct helper: it constructs geometry and does not manage transactions.
    feature = body.newObject('PartDesign::' + kind, kind)
    feature.Base = (base, ['Edge1'])
    setattr(feature, 'Radius' if kind == 'Fillet' else 'Size', 1.0)
    return feature


def state(doc, body):
    return (tuple(o.Name for o in doc.Objects), body.Tip.Name, body.Shape.Volume, doc.UndoCount)


def unchanged(doc, body, before):
    after = state(doc, body)
    check(after[:2] == before[:2] and after[3] == before[3], 'Objects, Body.Tip or undo stack changed')
    check(abs(after[2] - before[2]) < 1e-6, 'Volume changed')
    check(body.Shape.isValid(), 'Restored body is invalid')


def main():
    # Negative control proves that this test can detect the original bug.
    doc, body, base = fixture()
    try:
        before = state(doc, body)
        doc.openTransaction('outer edit')
        doc.openTransaction('incorrect nested helper')
        feature = blend(body, base, 'Fillet')
        doc.recompute()
        doc.commitTransaction()  # Original anti-pattern: helper commits early.
        try:
            raise RuntimeError('late application failure')
        except RuntimeError:
            doc.abortTransaction()
            doc.recompute()
        check(doc.getObject(feature.Name) is not None and state(doc, body) != before,
              'Negative control did not reproduce early-commit leakage on this runtime')
        checks.append('negative control reproduces nested-helper commit leakage')
    finally:
        App.closeDocument(doc.Name)

    doc, body, base = fixture()
    guard = Guard(App, OUT / 'checkpoints')
    try:
        original_path = OUT / 'original.FCStd'
        doc.saveAs(str(original_path))
        original_bytes = original_path.read_bytes()
        before = state(doc, body)
        for kind in ('Fillet', 'Chamfer'):
            def postcondition(_):
                raise RuntimeError('late application failure')
            try:
                guard.run(doc, kind, lambda _: blend(body, base, kind), validate=postcondition)
            except RuntimeError as exc:
                check(str(exc) == 'late application failure', 'Primary exception changed')
            else:
                raise AssertionError('Late failure was swallowed')
            unchanged(doc, body, before)
            check(guard.last_checkpoint.is_file(), 'Missing checkpoint')
            checkpoint_doc = App.openDocument(str(guard.last_checkpoint))
            try:
                restored_body = checkpoint_doc.getObject(body.Name)
                check(restored_body.Tip.Name == base.Name and restored_body.Shape.isValid(), 'Bad checkpoint topology')
                check(abs(restored_body.Shape.Volume-before[2]) < 1e-6, 'Bad checkpoint volume')
            finally:
                App.closeDocument(checkpoint_doc.Name)
                App.setActiveDocument(doc.Name)
            check(doc.FileName == str(original_path) and original_path.read_bytes() == original_bytes,
                  'Checkpoint changed the original saved file or save target')
            feature = guard.run(doc, kind, lambda _: blend(body, base, kind))
            name = feature.Name
            check(feature.Shape.isValid() and len(feature.Shape.Solids) == 1, 'Invalid native blend')
            check(feature.Base[0] == base and doc.UndoCount == before[3] + 1, 'Wrong base link or undo count')
            doc.undo(); doc.recompute()
            unchanged(doc, body, before)
            check(doc.getObject(name) is None, 'Undo retained feature')
            doc.redo(); doc.recompute()
            restored = doc.getObject(name)
            check(restored is not None and restored.Shape.isValid() and restored.Base[0] == base,
                  'Redo lost the native feature or base link')
            doc.undo(); doc.recompute()
            checks.append(kind.lower() + ': late-failure rollback, checkpoint reopen, one-step undo/redo')

        doc.openTransaction('foreign before first edit')
        foreign = App.getActiveTransaction()
        try:
            guard.run(doc, 'must refuse', lambda _: (_ for _ in ()).throw(AssertionError('Callback executed')))
        except OwnershipError:
            check(App.getActiveTransaction() == foreign, 'Foreign transaction was changed')
        else:
            raise AssertionError('Foreign transaction accepted')
        App.closeActiveTransaction(True)
        checks.append('foreign lazy transaction refused without closing it')

        for interrupt in (KeyboardInterrupt, SystemExit):
            def interrupted(_):
                blend(body, base, 'Fillet')
                doc.recompute()
                raise interrupt('interrupted')
            try:
                guard.run(doc, 'interruption', interrupted)
            except interrupt:
                unchanged(doc, body, before)
            else:
                raise AssertionError('Interruption swallowed')
        checks.append('KeyboardInterrupt and SystemExit roll back native edits')

        other = Guard(App, OUT / 'other-checkpoints')
        def nested(_):
            return other.run(doc, 'nested guard', lambda _: None)
        try:
            guard.run(doc, 'outer guard', nested)
        except OwnershipError:
            unchanged(doc, body, before)
        else:
            raise AssertionError('Nested guard accepted')
        checks.append('nested guard instance refused')

        errors = []
        def wrong_thread():
            try:
                guard.run(doc, 'worker thread', lambda _: None)
            except OwnershipError as error:
                errors.append(error)
        thread = threading.Thread(target=wrong_thread)
        thread.start(); thread.join()
        check(len(errors) == 1, 'Worker-thread call accepted')
        checks.append('worker-thread entry refused before native access')

        obstruction = OUT / 'not-a-directory'
        obstruction.write_text('fixture')
        try:
            Guard(App, obstruction).run(doc, 'checkpoint failure', lambda _: (_ for _ in ()).throw(AssertionError('Callback executed')))
        except FileExistsError:
            unchanged(doc, body, before)
        else:
            raise AssertionError('Checkpoint failure not propagated')
        checks.append('checkpoint I/O failure prevents callback execution')

        guard.run(doc, 'no changes', lambda _: None)
        unchanged(doc, body, before)
        check(App.getActiveTransaction() is None, 'No-op leaked a lazy transaction')
        checks.append('no-op closes lazy transaction without an undo entry')

        # Intentionally violate ownership LAST; all future guards must refuse.
        def malicious_helper(_):
            blend(body, base, 'Fillet')
            doc.recompute()
            doc.commitTransaction()
            raise RuntimeError('helper committed behind guard')
        try:
            guard.run(doc, 'ownership violation', malicious_helper)
        except RecoveryRequired as exc:
            check(exc.checkpoint.is_file() and isinstance(exc.primary, RuntimeError), 'Recovery information missing')
        else:
            raise AssertionError('Ownership loss was not surfaced')
        try:
            other.run(doc, 'unsafe retry', lambda _: None)
        except RecoveryRequired:
            checks.append('ownership loss surfaces recovery checkpoint and blocks every guard')
        else:
            raise AssertionError('Poisoned process accepted another edit')
    finally:
        App.closeDocument(doc.Name)


def abort_failure():
    """Fault injection into abort only; document and modeling stay native."""
    doc, body, base = fixture()
    class AbortFault:
        def __getattr__(self, name):
            return getattr(App, name)

        def closeActiveTransaction(self, abort=False):
            if abort:
                raise OSError('injected native abort failure')
            return App.closeActiveTransaction()

    guard = Guard(AbortFault(), OUT / 'checkpoints')
    try:
        def failing(_):
            blend(body, base, 'Fillet')
            doc.recompute()
            raise RuntimeError('primary operation failure')
        try:
            guard.run(doc, 'abort failure', failing)
        except RecoveryRequired as exc:
            check(str(exc.primary) == 'primary operation failure', 'Primary failure lost')
            check(str(exc.rollback) == 'injected native abort failure', 'Abort failure lost')
            check(exc.__cause__ is exc.primary and exc.checkpoint.is_file(), 'Cause or checkpoint lost')
        else:
            raise AssertionError('Abort failure did not poison the process')
        try:
            Guard(App, OUT/'new-guard').run(doc, 'unsafe retry', lambda _: None)
        except RecoveryRequired:
            checks.append('abort failure retains both exceptions and checkpoint; new guard blocked')
        else:
            raise AssertionError('Abort failure allowed further edits')
    finally:
        # Test harness cleanup only. The public guard offers no unsafe reset.
        App.closeActiveTransaction(True)
        App.closeDocument(doc.Name)


try:
    if os.environ.get('TRANSACTION_GUARD_CASE') == 'abort_failure':
        abort_failure()
    else:
        main()
    report = {'passed': True, 'freecad': list(App.Version()), 'occt': str(Part.OCC_VERSION),
              'checks': checks, 'gui_imported': 'FreeCADGui' in sys.modules}
except BaseException as exc:
    # Publish no personal paths or traceback in the uploadable report.
    report = {'passed': False, 'error_type': type(exc).__name__, 'error': str(exc), 'checks': checks}
(OUT / 'native-report.json').write_text(json.dumps(report, indent=2) + '\n')
print('TRANSACTION_GUARD_' + ('PASS' if report['passed'] else 'FAIL'), flush=True)
