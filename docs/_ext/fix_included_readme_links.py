"""
Sphinx extension to fix relative links in included README.md.

When README.md is included into index.md via {include}, the links inside
README.md are relative to the project root (e.g. [x](docs/xxx.md)).
But after inclusion, they should be relative to the docs/ directory
(e.g. [x](xxx.md)). This extension converts them at build time.
"""

import re
from sphinx.application import Sphinx
from sphinx.util.logging import getLogger

logger = getLogger(__name__)

def fix_readme_links(app, docname, source):
    """
    Modify the source content of docname if it contains an include of README.md.
    Replace markdown link patterns that start with 'docs/' with the plain filename.
    """
    # Only process the document that includes README.md (assume it's 'index')
    if docname != 'index':
        return

    content = source[0]

    # Check if this document includes README.md via MyST's {include}
    # MyST converts {include} to `.. include::` directive with appropriate path.
    # We check both common forms: ../README.md (from docs/index.md) and README.md (if in same dir)
    if '.. include:: ../README.md' not in content and '.. include:: README.md' not in content:
        return

    # Fix links: [text](docs/xxx.md) -> [text](xxx.md)
    # Correctly capture optional anchor: #section
    def fix_link(m):
        # m.group(1): filename with .md, m.group(2): anchor including #, or None
        anchor = m.group(2) or ''
        return f']({m.group(1)}{anchor})'

    new_content = re.sub(
        r'\]\(docs/([^)]+\.md)(#[^)]*)?\)',
        fix_link,
        content
    )

    if new_content != content:
        source[0] = new_content

def setup(app):
    app.connect('source-read', fix_readme_links)
    return {
        'version': '1.0.2',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }