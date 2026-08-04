"""Keep docutils system messages out of extracted content.

Background
----------

Sphinx's ``ApplySourceWorkaround`` transform (``sphinx.util.nodes.
apply_source_workaround``) sets ``rawsource = node.astext()`` on any
``TextElement``/``image``/``topic`` whose ``rawsource`` is empty.

MyST renders headings that live inside a directive (such as a sphinx-design
``{dropdown}``) as ``rubric`` nodes whose ``rawsource`` is left empty. Repeated
headings (e.g. the many ``#### Vanilla fixes:`` blocks in ``Whats-New.md``)
produce duplicate implicit targets, and docutils appends an ``INFO/1
system_message`` child to the rubric (via ``myst_parser``'s
``generate_heading_target`` -> ``document.note_implicit_target``). The
``astext()`` then includes that message, so the rubric's ``rawsource`` ends up
as::

    Vanilla fixes:D:\\Repos\\Phobos\\docs\\Whats-New.md:845: (INFO/1) Duplicate
    implicit target name: "vanilla fixes:".

The gettext builder later extracts ``rawsource`` as a translatable message, so
these warnings leak into the ``.pot``/``.po`` files.

This extension removes the system-message children from the doctree (Sphinx's
``FilterSystemMessages`` would remove them at write time anyway) and restores
the ``rawsource`` that ``ApplySourceWorkaround`` polluted with the message text.
"""

from __future__ import annotations

import re

from docutils import nodes

__version__ = '1.0.0'

# Matches the leading part of the message that docutils appends to ``rawsource``
# via ``system_message.astext()``: "<source>:<line>: (LEVEL/N) <message>".
_MESSAGE_START = re.compile(
    r'(?:[A-Za-z]:)?[\\/][^\r\n:]+:\d+: \((?:INFO|WARNING|ERROR|SEVERE)/\d+\) '
)


def _repair(app, doctree) -> None:
    # 1. Drop system-message children from content nodes.
    for node in list(doctree.findall(nodes.system_message)):
        parent = node.parent
        if parent is not None:
            parent.remove(node)

    # 2. Restore rawsource that ApplySourceWorkaround polluted with the
    #    system-message text.
    for node in doctree.findall(nodes.TextElement):
        raw = node.rawsource
        if not raw:
            continue
        match = _MESSAGE_START.search(raw)
        if match:
            node.rawsource = raw[:match.start()].rstrip()


def setup(app):
    app.connect('doctree-read', _repair)
    return {
        'version': __version__,
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
