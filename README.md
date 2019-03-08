# FX Calc
FX Position Calculator

On close of the application all values are saved in a file in `~/Library/Preferences/fxcalc`

# Compile mac app
To compile a macos app just do it like this:

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```

The App `FXCalc.app` is going to be generated. You can copy it to your Applications folder.

# dependencies
- Qt 5.12
- CMAKE 3.8
- https://exchangeratesapi.io

### inspiration 
- https://www.myfxbook.com/forex-calculators/position-size
- https://www.babypips.com/tools
