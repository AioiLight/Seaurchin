@powershell -NoProfile -ExecutionPolicy Unrestricted "$s=[scriptblock]::create((gc \"%~f0\"|?{$_.readcount -gt 1})-join\"`n\");&$s" %*&goto:eof

$ANGELSCRIPT_VER = "2.32.0"
$BOOST_VER = "1.68.0"
$ZLIB_VER = "1.2.11"
$LIBPNG_VER = "1.6.34"
$LIBJPEG_VER = "9c"
$LIBOGG_VER = "1.3.3"
$LIBVORBIS_VER = "1.3.6"
$FMT_VER = "4.1.0"
$SPDLOG_VER = "0.17.0"
$GLM_VER = "0.9.9.3"
$FREETYPE_VER = "2.9.1"

$BOOST_VER_UNDERLINE = $BOOST_VER.Replace(".","_")
$ZLIB_VER_NUM = $ZLIB_VER.Replace(".","")
$LIBPNG_VER_NUM = $LIBPNG_VER.Replace(".","")
$LIBPNG_VER_NUM2 = $LIBPNG_VER_NUM.Substring(0,2)

$BASE_PATH = pwd
$MSBUILD = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
$VS_TOOLS_VER = ls "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\" -NAME  | Select-Object -Last 1
$NMAKE = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\$VS_TOOLS_VER\bin\Hostx86\x86\nmake.exe"

[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12

Write-Host '================================================================================'
Write-Host '                                                                                '
Write-Host ' .d8888b.                                             888      d8b              '
Write-Host 'd88P  Y88b                                            888      Y8P              '
Write-Host 'Y88b.                                                 888                       '
Write-Host ' "Y888b.    .d88b.   8888b.  888  888 888d888 .d8888b 88888b.  888 88888b.      '
Write-Host '    "Y88b. d8P  Y8b     "88b 888  888 888P"  d88P"    888 "88b 888 888 "88b     '
Write-Host '      "888 88888888 .d888888 888  888 888    888      888  888 888 888  888     '
Write-Host 'Y88b  d88P Y8b.     888  888 Y88b 888 888    Y88b.    888  888 888 888  888     '
Write-Host ' "Y8888P"   "Y8888  "Y888888  "Y88888 888     "Y8888P 888  888 888 888  888     '
Write-Host '                                                                                '
Write-Host '   888888b.                     888    .d8888b.  888                            '
Write-Host '   888  "88b                    888   d88P  Y88b 888                            '
Write-Host '   888  .88P                    888   Y88b.      888                            '
Write-Host '   8888888K.   .d88b.   .d88b.  888888 "Y888b.   888888 888d888 8888b.  88888b. '
Write-Host '   888  "Y88b d88""88b d88""88b 888       "Y88b. 888    888P"      "88b 888 "88b'
Write-Host '   888    888 888  888 888  888 888         "888 888    888    .d888888 888  888'
Write-Host '   888   d88P Y88..88P Y88..88P Y88b. Y88b  d88P Y88b.  888    888  888 888 d88P'
Write-Host '   8888888P"   "Y88P"   "Y88P"   "Y888 "Y8888P"   "Y888 888    "Y888888 88888P" '
Write-Host '                                                                        888     '
Write-Host '                                                                        888     '
Write-Host '======================================================================= 888 ===='
Write-Host ''
Write-Host "Seaurchin BootStrap�ł�Seaurchin�̊J�����������I�ɍ\�z�����܂��B"
Read-Host '���s����ɂ� Enter �L�[�������Ă�������'

function noBuild($url,$name) {
  if (!(Test-Path "library\$name")) {
    if (!(Test-Path "library\$name.zip")) {
      Write-Host "** $name �̃f�[�^���擾���܂��B"
      Write-Host "$url"
      Invoke-WebRequest -Uri "$url" -OutFile "library\$name.zip"
    } else {
      Write-Host "** $name �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
      Write-Host ""
    }
    Write-Host "** $name ��W�J���܂��B"
    Expand-Archive -Path "library\$name.zip" -DestinationPath "library\$name" -force

    Write-Host "** $name �̓r���h�s�v�Ȃ̂Ńr���h�̓X�L�b�v���܂����B"
    Write-Host ""
  } else {
    Write-Host "** $name �͊��ɓW�J�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
}

Write-Host '================================================================================='
Write-Host ''
Write-Host "* ���C�u�����W�J��t�H���_�ƈꎞ�t�H���_�𐶐����܂��B"
Write-Host ''

New-Item library\ -ItemType Directory >$null 2>&1
New-Item tmp\ -ItemType Directory >$null 2>&1

Write-Host '================================================================================='
Write-Host ''
Write-Host "* ���\�z�ɕK�v�ȃR�}���h���������܂��B"
Write-Host ''

if (!(Test-Path "tmp\patch.zip")) {
  Write-Host "** patch�R�}���h�̃\�[�X�R�[�h���擾���܂��B"
  Write-Host "https://blogs.osdn.jp/2015/01/13/download/patch-2.5.9-7-bin.zip"
  Invoke-WebRequest -Uri "https://blogs.osdn.jp/2015/01/13/download/patch-2.5.9-7-bin.zip" -OutFile "tmp\patch.zip"
  Expand-Archive -Path "tmp/patch.zip" -DestinationPath "tmp"
  Write-Host ""
} else {
  Write-Host "** patch�R�}���h�͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}

$PATCH = Resolve-Path ".\tmp\bin\patch.exe"

Write-Host "================================================================================="
Write-Host ""
Write-Host "* �ˑ����C�u�����̎擾�E�r���h�����s���܂�"
Write-Host ""

if (!(Test-Path "library\angelscript")) {
  if (!(Test-Path "library\angelscript.zip")) {
    Write-Host "** AngelScript �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "https://www.angelcode.com/angelscript/sdk/files/angelscript_$ANGELSCRIPT_VER.zip"
    Invoke-WebRequest -Uri "https://www.angelcode.com/angelscript/sdk/files/angelscript_$ANGELSCRIPT_VER.zip" -OutFile "library\angelscript.zip"
  } else {
    Write-Host "** AngelScript �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** AngelScript ��W�J���܂��B"
  Expand-Archive -Path "library\angelscript.zip" -DestinationPath "library\angelscript" -force

  Write-Host "** AngelScript ���r���h���܂��B"
  cd library\angelscript\sdk\angelscript\projects\msvc2015
  &$PATCH angelscript.vcxproj "$BASE_PATH/bootstrap\angelscript.patch"
  &$MSBUILD angelscript.vcxproj /p:Configuration=Release
  cd "$BASE_PATH"

  Write-Host ""
} else {
  Write-Host "** AngelScript �͊��Ƀr���h�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}

if (!(Test-Path "library\boost")) {
  if (!(Test-Path "library\boost.zip")) {
    Write-Host "** Boost �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "https://dl.bintray.com/boostorg/release/$BOOST_VER/source/boost_$BOOST_VER_UNDERLINE.zip"
    Invoke-WebRequest -Uri "https://dl.bintray.com/boostorg/release/$BOOST_VER/source/boost_$BOOST_VER_UNDERLINE.zip" -OutFile "library\boost.zip"
  } else {
    Write-Host "** Boost �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** Boost ��W�J���܂��B"
  Expand-Archive -Path "library\boost.zip" -DestinationPath "library" -force
  cd "library"
  Rename-Item "boost_$BOOST_VER_UNDERLINE" "boost"
  cd "$BASE_PATH"

  Write-Host "** Boost ���r���h���܂��B"

  cd "library\boost"
  cmd /c "bootstrap.bat"
  cmd /c "b2 -j 4"
  cd "$BASE_PATH"

  Write-Host ""
} else {
  Write-Host "** Boost �͊��Ƀr���h�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}

if (!(Test-Path "library\zlib")) {
  if (!(Test-Path "library\zlib.zip")) {
    Write-Host "** zlib �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "https://zlib.net/zlib$ZLIB_VER_NUM.zip"
    Invoke-WebRequest -Uri "https://zlib.net/zlib$ZLIB_VER_NUM.zip" -OutFile "library\zlib.zip"
  } else {
    Write-Host "** zlib �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** zlib ��W�J���܂��B"
  Expand-Archive -Path "library\zlib.zip" -DestinationPath "library" -force
  cd "library"
  Rename-Item "zlib-$ZLIB_VER" "zlib"
  cd "$BASE_PATH"

  Write-Host "** zlib �̓r���h�s�v�Ȃ̂Ńr���h�̓X�L�b�v���܂����B"
  Write-Host ""
} else {
  Write-Host "** zlib �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}

if (!(Test-Path "library\libpng")) {
  if (!(Test-Path "library\libpng.zip")) {
    Write-Host "** libpng �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "http://ftp-osl.osuosl.org/pub/libpng/src/libpng$LIBPNG_VER_NUM2/lpng$LIBPNG_VER_NUM.zip"
    Invoke-WebRequest -Uri "http://ftp-osl.osuosl.org/pub/libpng/src/libpng$LIBPNG_VER_NUM2/lpng$LIBPNG_VER_NUM.zip" -OutFile "library\libpng.zip"
  } else {
    Write-Host "** libpng �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** libpng ��W�J���܂��B"
  Expand-Archive -Path "library\libpng.zip" -DestinationPath "library" -force
  
  cd "library"
  Rename-Item "lpng$LIBPNG_VER_NUM" "libpng"
  cd "$BASE_PATH"

  Write-Host "** libpng ���r���h���܂��B"
  
  cd "library\libpng\projects\vstudio"
  &$PATCH libpng\libpng.vcxproj         "$BASE_PATH\bootstrap\libpng.patch"
  &$PATCH pnglibconf\pnglibconf.vcxproj "$BASE_PATH\bootstrap\pnglibconf.patch"
  &$PATCH pngstest\pngstest.vcxproj     "$BASE_PATH\bootstrap\pngstest.patch"
  &$PATCH pngtest\pngtest.vcxproj       "$BASE_PATH\bootstrap\pngtest.patch"
  &$PATCH pngunknown\pngunknown.vcxproj "$BASE_PATH\bootstrap\pngunknown.patch"
  &$PATCH pngvlaid\pngvlaid.vcxproj     "$BASE_PATH\bootstrap\pngvalid.patch"
  &$PATCH zlib\zlib.vcxproj             "$BASE_PATH\bootstrap\zlib.patch"
  &$PATCH zlib.props                    "$BASE_PATH\bootstrap\zlib.props.patch"
  &$MSBUILD vstudio.sln /p:Configuration=Release
  cd "$BASE_PATH"

  Write-Host ""
} else {
  Write-Host "** libpng �͊��Ƀr���h�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}

if (!(Test-Path "library\libjpeg")) {
  if (!(Test-Path "library\libjpeg.zip")) {
    Write-Host "** libjpeg �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "https://www.ijg.org/files/jpegsr$LIBJPEG_VER.zip"
    Invoke-WebRequest -Uri "https://www.ijg.org/files/jpegsr$LIBJPEG_VER.zip" -OutFile "library\libjpeg.zip"
  } else {
    Write-Host "** libjpeg �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** libjpeg ��W�J���܂��B"
  Expand-Archive -Path "library\libjpeg.zip" -DestinationPath "library" -force

  Write-Host "** libjpeg ���r���h���܂��B"
  
  cd "library"
  Rename-Item "jpeg-$LIBJPEG_VER" "libjpeg"
  cd "$BASE_PATH"

  cd "library\libjpeg"
  &$NMAKE /f makefile.vs setup-v15
  &$PATCH --force jpeg.vcxproj         "$BASE_PATH\bootstrap\libjpeg.patch"
  &$MSBUILD jpeg.sln /p:Configuration=Release
  cd "$BASE_PATH"

  Write-Host ""
} else {
  Write-Host "** libjpeg �͊��Ƀr���h�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}

if (!(Test-Path "library\libogg")) {
  if (!(Test-Path "library\libogg.zip")) {
    Write-Host "** libogg �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "http://downloads.xiph.org/releases/ogg/libogg-$LIBOGG_VER.zip"
    Invoke-WebRequest -Uri "http://downloads.xiph.org/releases/ogg/libogg-$LIBOGG_VER.zip" -OutFile "library\libogg.zip"
  } else {
    Write-Host "** libogg �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** libogg ��W�J���܂��B"
  Expand-Archive -Path "library\libogg.zip" -DestinationPath "library" -force

  cd "library"
  Rename-Item "libogg-$LIBOGG_VER" "libogg"
  cd "$BASE_PATH"

  Write-Host "** libogg ���r���h���܂��B"
  
  cd "library\libogg\win32\VS2015"
  &$PATCH --force libogg_static.vcxproj "$BASE_PATH\bootstrap\libogg.patch"
  &$MSBUILD libogg_static.sln /p:Configuration=Release
  cd "$BASE_PATH"

  Write-Host ""
} else {
  Write-Host "** libogg �͊��Ƀr���h�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}

if (!(Test-Path "library\libvorbis")) {
  if (!(Test-Path "library\libvorbis.zip")) {
    Write-Host "** libvorbis �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "http://downloads.xiph.org/releases/vorbis/libvorbis-$LIBVORBIS_VER.zip"
    Invoke-WebRequest -Uri "http://downloads.xiph.org/releases/vorbis/libvorbis-$LIBVORBIS_VER.zip" -OutFile "library\libvorbis.zip"
  } else {
    Write-Host "** libvorbis �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** libvorbis ��W�J���܂��B"
  Expand-Archive -Path "library\libvorbis.zip" -DestinationPath "library" -force

  cd "library"
  Rename-Item "libvorbis-$LIBVORBIS_VER" "libvorbis"
  cd "$BASE_PATH"

  Write-Host "** libvorbis ���r���h���܂��B"
  
  cd "library\libvorbis\win32\VS2010"
  &$PATCH libvorbis\libvorbis_static.vcxproj         "$BASE_PATH\bootstrap\libvorbis.patch"
  &$PATCH libvorbisfile\libvorbisfile_static.vcxproj "$BASE_PATH\bootstrap\libvorbisfile.patch"
  &$PATCH vorbisdec\vorbisdec_static.vcxproj         "$BASE_PATH\bootstrap\vorbisdec.patch"
  &$PATCH vorbisenc\vorbisenc_static.vcxproj         "$BASE_PATH\bootstrap\vorbisenc.patch"
  &$PATCH libogg.props                               "$BASE_PATH\bootstrap\libogg.props.patch"
  &$MSBUILD vorbis_static.sln /p:Configuration=Release
 
  cd "$BASE_PATH"

  Write-Host ""
} else {
  Write-Host "** libogg �͊��Ƀr���h�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}


if (!(Test-Path "library\dxlib")) {
  if (!(Test-Path "library\dxlib.zip")) {
    Write-Host "** DxLib �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "http://dxlib.o.oo7.jp/DxLib/DxLib_VC3_19d.zip"
    Invoke-WebRequest -Uri "http://dxlib.o.oo7.jp/DxLib/DxLib_VC3_19d.zip" -OutFile "library\dxlib.zip"
  } else {
    Write-Host "** DxLib �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** DxLib ��W�J���܂��B"
  Expand-Archive -Path "library\dxlib.zip" -DestinationPath "library\dxlib" -force

  Write-Host "** DxLib �����l�[�����܂��B"
  
  cd "library\dxlib\DxLib_VC"
  Rename-Item "�v���W�F�N�g�ɒǉ����ׂ��t�@�C��_VC�p" "include"
  cd "$BASE_PATH"

  Write-Host ""
} else {
  Write-Host "** DxLib �͊��Ƀr���h�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}


if (!(Test-Path "library\spdlog")) {
  if (!(Test-Path "library\spdlog.zip")) {
    Write-Host "** spdlog �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "https://github.com/gabime/spdlog/archive/v$SPDLOG_VER.zip"
    Invoke-WebRequest -Uri "https://github.com/gabime/spdlog/archive/v$SPDLOG_VER.zip" -OutFile "library\spdlog.zip"
  } else {
    Write-Host "** spdlog �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** spdlog ��W�J���܂��B"
  Expand-Archive -Path "library\spdlog.zip" -DestinationPath "library" -force
  cd "library"
  Rename-Item "spdlog-$SPDLOG_VER" "spdlog"
  cd "$BASE_PATH"

  Write-Host "** spdlog �̓r���h�s�v�Ȃ̂Ńr���h�̓X�L�b�v���܂����B"

  Write-Host ""
} else {
  Write-Host "** spdlog �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}

if (!(Test-Path "library\fmt")) {
  if (!(Test-Path "library\fmt.zip")) {
    Write-Host "** fmt �̃\�[�X�R�[�h���擾���܂��B"
    Write-Host "https://github.com/fmtlib/fmt/releases/download/$FMT_VER/fmt-$FMT_VER.zip"
    Invoke-WebRequest -Uri "https://github.com/fmtlib/fmt/releases/download/$FMT_VER/fmt-$FMT_VER.zip" -OutFile "library\fmt.zip"
  } else {
    Write-Host "** fmt �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
    Write-Host ""
  }
  Write-Host "** fmt ��W�J���܂��B"
  Expand-Archive -Path "library\fmt.zip" -DestinationPath "library" -force

  cd "library"
  Rename-Item "fmt-$FMT_VER" "fmt"
  cd "$BASE_PATH"

  Write-Host "** fmt �̓r���h�s�v�Ȃ̂Ńr���h�̓X�L�b�v���܂����B"

  Write-Host ""
} else {
  Write-Host "** fmt �͊��Ɏ擾�ςȂ̂Ŗ������܂����B"
  Write-Host ""
}

noBuild "http://us.un4seen.com/files/bass24.zip" "bass24"
noBuild "http://us.un4seen.com/files/z/0/bass_fx24.zip" "bass24_fx"
noBuild "http://us.un4seen.com/files/bassmix24.zip" "bass24_mix"

noBuild "https://github.com/ubawurinna/freetype-windows-binaries/releases/download/v$FREETYPE_VER/freetype-$FREETYPE_VER.zip" "freetype"
noBuild "https://github.com/mayah/tinytoml/archive/master.zip" "tinytoml"
noBuild "https://github.com/g-truc/glm/releases/download/$GLM_VER/glm-$GLM_VER.zip" "glm"

# Wacom�@�\���p�~�����܂ŗՎ��[�u
noBuild "https://github.com/denisidoro/xWacom/archive/master.zip" "wacom"

pause