define reload
mon reset
#shell make flash
load
end

target remote localhost:3333
load

