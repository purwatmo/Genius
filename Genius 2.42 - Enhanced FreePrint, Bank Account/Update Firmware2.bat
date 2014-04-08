echo off
cls
echo =====================
echo Generate Update Files
echo =====================
set /p Ver=Enter Version Number, press Enter:  

if %Ver%=="" GOTO Exit

Proceed:
MD E:\Hanindo\Source~1\Final\sent\Latest\Mega128
MD E:\Hanindo\Source~1\Final\sent\Latest\Mega162
copy e:\Hanindo\Source~1\Genius~1.21D\Mega128\default\master4.hex E:\Hanindo\Source~1\Final\sent\Latest%Ver%\Mega128\Mega128.hex
copy e:\Hanindo\Source~1\Genius~1.21D\Mega128\default\master4.eep E:\Hanindo\Source~1\Final\sent\Latest%Ver%\Mega128\Mega128.eep
copy e:\Hanindo\Source~1\Genius~1.21D\Mega162\default\no6.hex E:\Hanindo\Source~1\Final\sent\Latest%Ver%\Mega162\Mega162.hex
copy e:\Hanindo\Source~1\Genius~1.21D\Mega162\default\no6.eep E:\Hanindo\Source~1\Final\sent\Latest%Ver%\Mega162\Mega162.eep
copy e:\Hanindo\Source~1\Genius~1.21D\Version.txt E:\Hanindo\Source~1\Final\sent\Latest%Ver%\Version.txt
START E:\Hanindo\Source~1\Final\sent

Exit: