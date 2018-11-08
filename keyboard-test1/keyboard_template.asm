; we need pins PCINT0 & PCINT1 to control counter U3 (PCINT0 sends CLK signal to increase the counter, PCINT1 sends RST signal to reset one)
; similarly, pins PCINT2 & PCINT3 control counter U5 (PCINT2 - CLK, PCINT3 - RST)
; pin 3 will be reading the data

; counter U3 should count up to 4 (since we have 4 rows of keys)
; counter U5 should count up to 3 (since we have only 3 columns of keys)

; the above is valid until one short-circuits CO (carry) and MR (reset) pins of both counters:
; in that case both counters should count until the maximum is reached; in case of 74HC4017 counters this number is 10
; once 10 is reached - the CO pin will have HIGH value (roughly: +5V) and this will flow to the MR pin, which will reset the state
; of a counter to 0.

; the whole keyboard works like this:
;
; 1. increase the number on the row counter (U3)
; 1a. this will set pins of a counter to represent value 1 in binary (Q0 = HIGH, others = LOW)
; 1b. this value represents an address, which is then demultiplexed by U4 and the corresponding pin is set to HIGH (Y0 = HIGH, others = LOW)
; 2. increase the number on the column counter (U5)
; 2a. this will work exactly as for columns, e.g. the pin Q0 of U5 will be HIGH -> pin Y0 of U6 will be also HIGH, other pins in both U5 and U6 will be LOW
; 2b. the difference with columns, however, starts at the multiplexer U2 - the address pins A, B and C will determine which of the input pins X0..X7 will be connected to the output X
; 3. if the corresponding key (in row 1 and column 1) is pressed - the value on pin X of a multiplexer U2 should be HIGH
; 3a. this value will be received by pin PCINT4 of an MCU
; 3b. in order to prevent any contact fizz'n'buzz, we should set a delay of, say, 10 milliseconds and then ensure the signal is still the same; generally, we can just wait 10 millis and then simply take the value of a PCINT4
;
; note: for 74HC238 to work as expected, E3 pin should be set to HIGH, hence I've connected them to the RESET pin of the MCU (PCINT5), which should be always set to HIGH

; pseudo-code:
;
; set up pins for MCU: PCINT0, PCINT1, PCINT2, PCINT3, PCINT5 - output, PCINT4 - input
; set up demultiplexers: write HIGH to PCINT5
; set up counter values: rows to 0, cols to 0
; start scanning loop: write HIGH to PCINT0 and HIGH to PCINT2; wait for 2 milliseconds; write LOW to PCINT0 and LOW to PCINT2
; wait for 10 milliseconds; read the value on the PCINT4; 
;    if this is HIGH - *send data to receiver about key(1, 1) being pressed down*;
;    if it is low - *send data to receiver about key(1, 1) being released* 
; ^^^ wrong! ideally - store the values in the array and compare them and send the "keyup" event ONLY when previous state was HIGH 
; ^^^ and the new state is LOW and, vice vesa, "keydown" when old state is LOW and the new state is HIGH
; increment the value of cols counter
; compare the value of cols counter with COLS_NUM (3) - if less: jump to the loop start
; *this line being reached means cols counter is greater or equal to COLS_NUM* set counter cols to 0; increment counter rows
; compare value of rows counter with ROWS_NUM (4) - if less: jump to the loop start
; *this line being reached means BOTH cols counter is greater or equal to COLS_NUM and rows counter is greater or equal to ROWS_NUM* set counter rows to 0
; jump to the loop start