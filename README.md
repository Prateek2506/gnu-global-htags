___________________________________
|      |  |  |     |  _  |     |  |
|  |___|  |  |  |  |    _|  |  |  |    GNU GLOBAL source code tagging system
|  |   |  |  |  |  |     |     |  |
|  ~~  |   ~~|     |  ~  |  |  |   ~~|          for all hackers.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is a repo for recording my changes to the official GNU Global Source Code tagging system.
The existing package for HTAGS makes it difficult to differentiate between separate code blocks.
These are some changes that I made to the source code so as to make it more readable and elegant for browser view.

## src2html.c
This file is modified so that it counts the opening and closing braces and assigns different color to each block.

## style.js
This file is newly added to give collapse and expand feature to each code block.
