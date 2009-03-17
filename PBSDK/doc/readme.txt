
Пока никакой документации нет. См. usr/include/inkview.h и sources/inkdemo.

Пути, которые видит программа (такие же на реальном устройстве):
/mnt/ext1 - внутренняя флешка
/mnt/ext2 - SD-карта
/ebrmain/config - системные файлы конфигурации
/mnt/ext1/system/config - пользовательские файлы конфигурации
/ebrmain/fonts - системные шрифты
/mnt/ext1/system/fonts - пользовательские шрифты
/ebrmain/languages - файлы локализации
/mnt/ext1/system/languages - пользовательские файлы локализации
/ebrmain/themes/default.pbt - скин по умолчанию
/mnt/ext1/system/themes - пользовательские скины

Клавиши:
Left, Right, Up, Down - джойстик
Enter - центральная клавиша (OK)
ESC - power
+,-
F1 - Music
F2 - Menu
F3 - Back
F4 - Trash

Команда pbres -c file.c file1.bmp file2.bmp ... преобразует BMP-файлы в исходник С
в формате ibitmap, при этом цвет RGB(128,128,64) считается прозрачным.

Если не компилируется/не запускается: проверьте, что в Windows\System32 
есть файл cygpb1.dll, а переменная окружения POCKETBOOKSDK указывает на
каталог с SDK.
