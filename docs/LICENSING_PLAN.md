# Licensing plan — review required

**Status: not a license grant.** The repository owner has not yet approved a final license for NexSolid's original starter code or future contributions. This document explains what must be checked before importing modified OCCT source.

## Current repository
The starter project currently **uses OCCT as an external dependency**. It does not yet contain the user's full modified OCCT source tree. Do not describe the entire current repository as an OCCT fork without first importing and auditing that source.

## When importing modified OCCT
1. Identify the precise upstream OCCT release/commit and record its source URL.
2. Preserve upstream copyright notices, license texts, and applicable Open CASCADE additional exception.
3. Keep required modification notices, including dates, in modified library files.
4. Record which files are unchanged upstream, modified upstream, or independently authored.
5. Review the exact OCCT license and exception bundled with the imported version. Do not substitute a generic license file or claim that all new files automatically inherit the exception.
6. Review the proposed license and contribution terms for original NexSolid files, including contributor copyright and any future commercial licensing intentions.
7. Check source-distribution, relinking, and notice obligations for any binaries or applications distributed with the modified library.

The LGPL generally permits commercial use and redistribution under its terms. Publishing source on an obscure public repository does not remove recipients' rights or automatically meet all distribution obligations.

**Before publishing a full modified OCCT tree or accepting substantial external code, obtain a qualified open-source licensing review.**
