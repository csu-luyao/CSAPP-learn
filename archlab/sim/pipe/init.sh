make drivers
make psim VERSION=full
make VERSION=full
./psim -t sdriver.yo
./psim -t ldriver.yo
make drivers
../misc/yis sdriver.yo
