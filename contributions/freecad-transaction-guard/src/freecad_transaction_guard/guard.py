# SPDX-License-Identifier: LGPL-2.1-or-later
"""Derived from Neutron 101's Forma/bridge.py; see NOTICE for provenance.

Callbacks must edit only the supplied document and must not manage transactions.
FreeCAD is injected so importing this package does not import native libraries.
"""
import math
import threading
import uuid
import zipfile
from pathlib import Path


class GeometryError(RuntimeError):
    pass


class OwnershipError(RuntimeError):
    pass


class RecoveryRequired(RuntimeError):
    """Native state is uncertain. Restart the process and inspect the checkpoint."""
    def __init__(self, checkpoint, primary, rollback=None):
        self.checkpoint = checkpoint
        self.primary = primary
        self.rollback = rollback
        super().__init__('Transaction state is uncertain; recover from ' + str(checkpoint))


def validate_document(doc):
    for obj in doc.Objects:
        if 'Invalid' in obj.State:
            raise GeometryError('Invalid feature: ' + obj.Name)
        shape = getattr(obj, 'Shape', None)
        if shape is None or shape.isNull():
            continue
        if not shape.isValid():
            raise GeometryError('Invalid shape: ' + obj.Name)
        if shape.Solids and (not math.isfinite(shape.Volume) or shape.Volume <= 0):
            raise GeometryError('Nonpositive or nonfinite solid volume: ' + obj.Name)


# FreeCAD active transactions are application-wide, so ownership is shared by
# ALL guards in this module. An uncertain result blocks all subsequent edits.
_busy = False
_poisoned = None


class Guard:
    def __init__(self, app, checkpoint_dir):
        self.app = app
        self.checkpoint_dir = Path(checkpoint_dir)
        self.last_checkpoint = None

    def _preflight(self, doc):
        if threading.current_thread() is not threading.main_thread():
            raise OwnershipError('Call on the FreeCAD main thread')
        if _poisoned is not None:
            raise RecoveryRequired(_poisoned, OwnershipError('Process requires recovery'))
        if _busy:
            raise OwnershipError('Another guard is executing an edit')
        if self.app.activeDocument() != doc:
            raise OwnershipError('The supplied document must be active')
        if doc.UndoMode != 1:
            raise OwnershipError('Enable native UndoMode=1 before using the guard')
        if self.app.getActiveTransaction() is not None or any(
                d.HasPendingTransaction for d in self.app.listDocuments().values()):
            raise OwnershipError('An existing native transaction must finish first')

    def _owned(self, identity):
        if identity is None or self.app.getActiveTransaction() != identity:
            raise OwnershipError('Callback changed native transaction ownership')

    @staticmethod
    def _recompute(doc):
        if doc.recompute() is False:
            raise GeometryError('Native recompute reported failure')

    def run(self, doc, label, operation, *, validate=None):
        """Checkpoint, execute, validate and commit one native undo step.

        `validate` is an additional READ-ONLY application postcondition. Checkpoints are
        retained on success and failure. Recovery is explicit, never an automatic
        document swap. A callback may return a native feature owned by `doc`.
        """
        global _busy, _poisoned
        self._preflight(doc)
        if not isinstance(label, str) or not label.strip() or not callable(operation):
            raise ValueError('A nonempty label and callable operation are required')
        if validate is not None and not callable(validate):
            raise ValueError('validate must be callable')
        _busy = True
        identity = None
        opened = False
        checkpoint = None
        self.last_checkpoint = None
        try:
            self.checkpoint_dir.mkdir(parents=True, exist_ok=True)
            checkpoint = self.checkpoint_dir / (uuid.uuid4().hex + '.FCStd')
            # A failed checkpoint prevents callback execution. saveCopy leaves
            # the user's current document filename unchanged.
            saved = doc.saveCopy(str(checkpoint))
            if saved is False or not checkpoint.is_file():
                raise OSError('Native checkpoint was not saved')
            with zipfile.ZipFile(checkpoint) as archive:
                if 'Document.xml' not in archive.namelist():
                    raise OSError('Checkpoint lacks Document.xml')
            self.last_checkpoint = checkpoint
            doc.openTransaction(label)
            opened = True
            identity = self.app.getActiveTransaction()
            self._owned(identity)
            result = operation(doc)
            self._owned(identity)
            if self.app.activeDocument() != doc:
                raise ValueError('Callback changed the active document')
            self._recompute(doc)
            validate_document(doc)
            if validate is not None:
                validate(doc)
            self._owned(identity)
            if self.app.activeDocument() != doc:
                raise ValueError('Validator changed the active document')
            self.app.closeActiveTransaction()
            opened = False
            if self.app.getActiveTransaction() is not None or doc.HasPendingTransaction:
                raise OwnershipError('Native commit did not close its transaction')
            return result
        except BaseException as primary:
            if opened:
                try:
                    # Never abort a foreign transaction if a callback broke
                    # its contract. The checkpoint is the recovery boundary.
                    self._owned(identity)
                    # Closing at application scope also handles a lazy
                    # transaction whose callback failed before its first edit.
                    self.app.closeActiveTransaction(True)
                    if self.app.getActiveTransaction() is not None or doc.HasPendingTransaction:
                        raise OwnershipError('Native abort did not close its transaction')
                    self._recompute(doc)
                    validate_document(doc)
                except BaseException as rollback:
                    _poisoned = checkpoint
                    raise RecoveryRequired(checkpoint, primary, rollback) from primary
            elif isinstance(primary, OwnershipError) or self.app.getActiveTransaction() is not None:
                _poisoned = checkpoint
                raise RecoveryRequired(checkpoint, primary) from primary
            raise
        finally:
            _busy = False
