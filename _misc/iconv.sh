#!/bin/bash
for f in *.cpp; do iconv -f Windows-1250 -t UTF-8 $f > $f.utf8; done
for f in *.h; do iconv -f Windows-1250 -t UTF-8 $f > $f.utf8; done
