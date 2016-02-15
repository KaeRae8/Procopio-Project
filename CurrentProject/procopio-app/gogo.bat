@if exist upgrade\new_project (
  @xcopy /s /y upgrade\new_project\* .
  @rmdir /s /q upgrade\new_project
  @echo * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  @echo                           PROJECT UPGRADED                           
  @echo * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
)
cmd /C bin\windows\gogo.exe -bin bin\windows -src_path "bin;libraries/slag/standard" %1 %2 %3 %4 %5 %6

