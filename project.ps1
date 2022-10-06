function build()
{
  New-Item -ItemType Directory -Path "build" -ErrorAction Ignore
  Push-Location "build"
  cmake .. -G"Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
  cmake --build .
  Pop-Location
}

function clean
{
  Remove-Item -Path "build" -Recurse -Force
}

function run()
{
  build
  ./build/mylisp
}

function clean
{
  Remove-Item -Path "build" -Recurse -Force
}

function run($param)
{
  build
  ./build/mylisp $param
}
if ($args[0])
{
  switch($args[0])
  {
    "build"
    {
      build
    }
    "clean"
    {clean
    }
    "run"
    {
      run($args[1])
    }
    default
    {Write-Output "command not valid"
    }
  }
}
  
