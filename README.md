# FX Calc
FX Position Calculator

This application is a tool to calculate forex position sizes as lots and quantity. Also it shows how much margin is required to open the position.

# Compile mac app
To compile a macos app just do it like this:

```
$ chmod +x build.sh
$ ./build.sh release
```

The application bundle `FXCalc.app` is going to be generated. You can copy it to your `/Applications` folder.

# dependencies
- Qt 5.12
- CMAKE 3.8

## Note
On close of the application all values are saved in a file in `~/Library/Preferences/fxcalc`

### inspiration 
- https://www.myfxbook.com/forex-calculators/position-size
- https://www.babypips.com/tools
