#!/usr/bin/python
import os
import cgi

form = cgi.FieldStorage()
fileitem = form['filename']
if fileitem.filename:
    print(fileitem.filename + "is uploading")
    fn = os.path.basename(fileitem.filename)
    open(fn, 'wb').write(fileitem.file.read())
