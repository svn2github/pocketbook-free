Простенькая обертка вокруг inkview для Python.


Необходимые инструменты:

1. PBSDK 12.5
   Адрес: http://sourceforge.net/projects/pocketbook-free/

2. SWIG 1.3.40 (не уверен, что старые версии поддерживают python3)
   Адрес: http://swig.org

3. Python 3.1  (для запуска на 2.6 нужно использовать 
                PyInt_*, PyStr_* вместо PyLong_* и PyUnicode_*, 
                а добавлять дополнительно макросы и потом тестить - лень)
   Адрес: http://python.org
   Его нужно собрать либо под PBSDK, либо найти, либо сгенерировать 
   libpython31.a, которая необходима для сборки модуля.
 

E-mail: evgenybf@users.sourceforge.net 
http://www.the-ebook.org: ufff

