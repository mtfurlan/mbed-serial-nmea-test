#include "mbed.h"
#include "minmea.h"

DigitalOut led(LED1);

//pin names
//mbed-os/targets/TARGET_NORDIC/TARGET_NRF5x/TARGET_NRF52/TARGET_MCU_NRF52840/TARGET_NRF52840_DK/PinNames.h


#define INDENT_SPACES "    "
RawSerial pc(P0_6, P0_8);

RawSerial uart(P0_26, P0_27);
//Serial uart(P0_26, P0_27);

void parse_nmea(char* line) {
    switch (minmea_sentence_id(line, false)) {
        case MINMEA_SENTENCE_RMC: {
            struct minmea_sentence_rmc frame;
            if (minmea_parse_rmc(&frame, line)) {
                pc.printf(INDENT_SPACES "$xxRMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
                        frame.latitude.value, frame.latitude.scale,
                        frame.longitude.value, frame.longitude.scale,
                        frame.speed.value, frame.speed.scale);
                pc.printf(INDENT_SPACES "$xxRMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
                        minmea_rescale(&frame.latitude, 1000),
                        minmea_rescale(&frame.longitude, 1000),
                        minmea_rescale(&frame.speed, 1000));
                pc.printf(INDENT_SPACES "$xxRMC floating point degree coordinates and speed: (%f,%f) %f\n",
                        minmea_tocoord(&frame.latitude),
                        minmea_tocoord(&frame.longitude),
                        minmea_tofloat(&frame.speed));
            } else {
                pc.printf(INDENT_SPACES "$xxRMC sentence is not parsed\n");
            }
        } break;

        case MINMEA_SENTENCE_GGA: {
            struct minmea_sentence_gga frame;
            if (minmea_parse_gga(&frame, line)) {
                pc.printf(INDENT_SPACES "$xxGGA: fix quality: %d\n", frame.fix_quality);
            } else {
                pc.printf(INDENT_SPACES "$xxGGA sentence is not parsed\n");
            }
        } break;

        case MINMEA_SENTENCE_GST: {
            struct minmea_sentence_gst frame;
            if (minmea_parse_gst(&frame, line)) {
                pc.printf(INDENT_SPACES "$xxGST: raw latitude,longitude and altitude error deviation: (%d/%d,%d/%d,%d/%d)\n",
                        frame.latitude_error_deviation.value, frame.latitude_error_deviation.scale,
                        frame.longitude_error_deviation.value, frame.longitude_error_deviation.scale,
                        frame.altitude_error_deviation.value, frame.altitude_error_deviation.scale);
                pc.printf(INDENT_SPACES "$xxGST fixed point latitude,longitude and altitude error deviation"
                       " scaled to one decimal place: (%d,%d,%d)\n",
                        minmea_rescale(&frame.latitude_error_deviation, 10),
                        minmea_rescale(&frame.longitude_error_deviation, 10),
                        minmea_rescale(&frame.altitude_error_deviation, 10));
                pc.printf(INDENT_SPACES "$xxGST floating point degree latitude, longitude and altitude error deviation: (%f,%f,%f)",
                        minmea_tofloat(&frame.latitude_error_deviation),
                        minmea_tofloat(&frame.longitude_error_deviation),
                        minmea_tofloat(&frame.altitude_error_deviation));
            } else {
                pc.printf(INDENT_SPACES "$xxGST sentence is not parsed\n");
            }
        } break;

        case MINMEA_SENTENCE_GSV: {
            struct minmea_sentence_gsv frame;
            if (minmea_parse_gsv(&frame, line)) {
                pc.printf(INDENT_SPACES "$xxGSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
                pc.printf(INDENT_SPACES "$xxGSV: sattelites in view: %d\n", frame.total_sats);
                for (int i = 0; i < 4; i++)
                    pc.printf(INDENT_SPACES "$xxGSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
                        frame.sats[i].nr,
                        frame.sats[i].elevation,
                        frame.sats[i].azimuth,
                        frame.sats[i].snr);
            } else {
                pc.printf(INDENT_SPACES "$xxGSV sentence is not parsed\n");
            }
        } break;

        case MINMEA_SENTENCE_VTG: {
           struct minmea_sentence_vtg frame;
           if (minmea_parse_vtg(&frame, line)) {
                pc.printf(INDENT_SPACES "$xxVTG: true track degrees = %f\n",
                       minmea_tofloat(&frame.true_track_degrees));
                pc.printf(INDENT_SPACES "        magnetic track degrees = %f\n",
                       minmea_tofloat(&frame.magnetic_track_degrees));
                pc.printf(INDENT_SPACES "        speed knots = %f\n",
                        minmea_tofloat(&frame.speed_knots));
                pc.printf(INDENT_SPACES "        speed kph = %f\n",
                        minmea_tofloat(&frame.speed_kph));
           } else {
                pc.printf(INDENT_SPACES "$xxVTG sentence is not parsed\n");
           }
        } break;

        case MINMEA_SENTENCE_ZDA: {
            struct minmea_sentence_zda frame;
            if (minmea_parse_zda(&frame, line)) {
                pc.printf(INDENT_SPACES "$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d\n",
                       frame.time.hours,
                       frame.time.minutes,
                       frame.time.seconds,
                       frame.date.day,
                       frame.date.month,
                       frame.date.year,
                       frame.hour_offset,
                       frame.minute_offset);
            } else {
                pc.printf(INDENT_SPACES "$xxZDA sentence is not parsed\n");
            }
        } break;

        case MINMEA_INVALID: {
            pc.printf(INDENT_SPACES "$xxxxx sentence is not valid\n");
        } break;

        default: {
            pc.printf(INDENT_SPACES "$xxxxx sentence is not parsed\n");
        } break;
    }
}

volatile int len = 0;
volatile bool tryParse = 0;
//volatile char line[512];
//TODO: is +1 necessary
//TODO: is it cool to write to a char* in an ISR
char line[MINMEA_MAX_LENGTH+1];
void handleGNSSChar() {
    while(uart.readable()){
        char c;
        c = uart.getc();
        if(c == '\n' || c == '\r' || len == sizeof(line)-1) {
            if(len == 0) {
                continue;
            }
            line[len++] = c;
            tryParse = true;
            line[len] = '\0';
        } else {
            line[len++] = c;
        }
    }
}

// main() runs in its own thread in the OS
int main() {
    uart.baud(4800);
    uart.attach(handleGNSSChar);
    pc.printf("HELLO WORLD 6\n");
    uart.printf("HELLO WORLD\n");
    while (true) {
        if(tryParse) {
            pc.printf("newline! (or too many characters) parsing\n");
            for(int i=0; i<len; ++i) {
                pc.printf("%3d", i);
            }
            pc.printf("\n");
            for(int i=0; i<len; ++i) {
                pc.printf(" %02X", line[i]);
            }
            pc.printf("\n");
            for(int i=0; i<len; ++i) {
                if(line[i] == '\r') {
                    pc.printf(" \r");
                } else if(line[i] == '\n') {
                    pc.printf(" \n");
                } else {
                    pc.printf("%3c", line[i]);
                }
            }
            pc.printf("\n");
            pc.printf("###\n'%s'\n###\n", line);
            //I hope this loops over each chunk
            char* newline;
            char* start = line;
            while((newline = strchr(start, '\r'))) {
                //TODO: misses the second part of line when there are multiple
                //  lines in it
                newline[0] = '\0';

                pc.printf("start addr: %p\n", start);
                pc.printf("newline addr: %p\n", newline);
                pc.printf("##'%s'##\n", start);
                // pc.printf("\nfirst few chars after newline\n");
                // for(int i=0; i<5; ++i) {
                //     pc.printf(" %02X", newline[i]);
                // }
                // pc.printf("\n");

                parse_nmea(start);

                newline++; //It was pointing to null
                //Now ensure that the next character is CR
                if(newline[0] == '\n') {
                    newline++;
                }
                start = newline;
            }
            len = 0;
            line[len] = '\0';
            tryParse = false;
        }
        led = !led;
        //pc.printf("loop hi WORLD %d\n", len);
        //wait(0.5f);
    }
}
