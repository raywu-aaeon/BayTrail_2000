@REM @file
@REM   Windows batch file to setup a Manifest Generation Tool environment
@REM

@REM
@REM $Date: 04/15/2013 KimiWu $
@REM

@REM ##############################################################
@REM # You should not have to modify anything below this line
@REM #

@ECHO OFF

SET OEM_KEY_HASH_1=

FOR /F "usebackq tokens=1" %%I IN ("%1") DO (
  IF NOT "%%I" == "" (
      SET OEM_KEY_HASH_1=%%I
  )
)

FOR /F "usebackq eol=# tokens=1-3 delims=:" %%I IN ("%2") DO (
  IF "%%I" == "FUSE_FILE_SECURE_BOOT_EN" (
      ECHO %%I:01:%%K> %BUILD_DIR%\%~nx2
  ) ELSE (
      IF "%%I" == "FUSE_FILE_OEM_KEY_HASH_1" (
        ECHO %%I:%OEM_KEY_HASH_1%:%%K>> %BUILD_DIR%\%~nx2
      ) ELSE (
        ECHO %%I:%%J:%%K>> %BUILD_DIR%\%~nx2
      )
  )
)

@ECHO ON