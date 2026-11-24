# SPDX-License-Identifier: LGPL-2.1-or-later
"""Single-owner, checkpointed FreeCAD document transactions."""
from .guard import GeometryError, Guard, OwnershipError, RecoveryRequired, validate_document

__all__ = ['GeometryError', 'Guard', 'OwnershipError', 'RecoveryRequired', 'validate_document']
