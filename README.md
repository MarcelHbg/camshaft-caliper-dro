# camshaft-caliper-dro

## CMD definitions (CamMeasurementMain)
all commands has to be finished with a New Line character

### start
- starts a complete measurement of a cam 
- split by a space there can be entered the distance in degrees between two measurements
- if no distance is given 1 degree is default 
- example: start 0.1

### read
- reads a sample of values from the caliper an displays the average 
- Amount of samples is defined by SAMPLE_NUM

### test
- reads five times from the caliper with a pause of 3sec between to test the caliper reading

### rotate
- rotates the camshaft a given value in degrees
- example: rotate 2.5 -> (rotates 2.5 degrees)

## Used Hardware
### Caliper
[Herbst 150mm Caliper](https://amzn.eu/d/71CtLWt)
