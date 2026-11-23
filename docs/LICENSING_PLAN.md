# Licensing plan — review required

**Status:** The repository now includes a [LICENSE](../LICENSE) covering the project owner's original NexSolid starter code under LGPL 2.1 with the Open CASCADE additional exception. The [full LGPL text](../LICENSE_LGPL_21.txt) and [OCCT exception](../OCCT_LGPL_EXCEPTION.txt) are included. The exact imported, modified OCCT tree has **not** yet been checked; a source-provenance and licensing review is still required before importing it.

## Current repository
The starter project currently **uses OCCT as an external dependency**. It does not yet contain the user's full modified OCCT source tree. Do not describe the entire current repository as an OCCT fork without first importing and auditing that source.

## When importing modified OCCT
1. Identify the precise upstream OCCT release/commit and record its source URL.
2. Preserve upstream copyright notices, license texts, and applicable Open CASCADE additional exception.
3. Keep required modification notices, including dates, in modified library files.
4. Record which files are unchanged upstream, modified upstream, or independently authored.
5. Review the exact OCCT license and exception bundled with the imported version. Do not substitute a generic license file or claim that all new files automatically inherit the exception.
6. Review file-level licenses and contribution terms for original NexSolid files, including contributor copyright and any future commercial licensing intentions. The repository's `LICENSE` describes the intended default for the owner's starter code.
7. Check source-distribution, relinking, and notice obligations for any binaries or applications distributed with the modified library.

The LGPL generally permits commercial use and redistribution under its terms. Publishing source on an obscure public repository does not remove recipients' rights or automatically meet all distribution obligations.

**Before publishing a full modified OCCT tree or accepting substantial external code, obtain a qualified open-source licensing review.**
