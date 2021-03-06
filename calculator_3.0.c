#define LEDs 8
#define tables 5

#define inputs 18


const struct {
  int LED_pins[LEDs] = {2, 3, 4, 5, 6, 7, 8, 9};
  int table_pins[tables] = {A5, A4, A3, A2, A1};
  int digital_mapping[21][8] = {
    {1, 1, 1, 1, 1, 1, 0, 0}, {0, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 0, 1, 1, 0, 1, 0}, {1, 1, 1, 1, 0, 0, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0}, {1, 0, 1, 1, 0, 1, 1, 0},
    {1, 0, 1, 1, 1, 1, 1, 0}, {1, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 0}, {1, 1, 1, 1, 0, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 0, 1}, {0, 1, 1, 0, 0, 0, 0, 1},
    {1, 1, 0, 1, 1, 0, 1, 1}, {1, 1, 1, 1, 0, 0, 1, 1},
    {0, 1, 1, 0, 0, 1, 1, 1}, {1, 0, 1, 1, 0, 1, 1, 1},
    {1, 0, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 0, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0}
  };
} Out_INFO;


const struct {
  int input_pin = A0;
  char events_mapping[inputs] =
  { 'C', '/', '9', '8', '7',
    '4', '5', '6', '*', 'R',
    '-', '3', '2', '1',
    '.', '0', '+', '='
  };
  int min_value = 110;
  int intervals[inputs] = {
    1003, 961, 923, 854, 795, 744, 700, 658, 602, 554, 513, 469, 431, 368, 320, 284, 243, 167
  };
} In_INFO;


struct Operation {
  int current_stage;       // a + b = c      'a' is stage0, '+' and 'b' is stage1, '=' is stage2
  int display_stage;
  double operand[3];       // a b c
  char operate;            // '?' '+' '-' '*' '/'
  int dot_position[3];     // 18.56 is 1 8. 5 6, since '6' is position 0, then  dot_position=2
};

char eventHandler(char event);
void arithmeticEvent(char event, struct Operation* o);
void uploadBuffer(struct Operation* o);
void lightUp();
char check_Input();
long Log(long base, long x);
long Pow(long base , long power);


int buffer[] = {0, 20, 20, 20, 20};




void setup() {
  for (int i = 0; i < LEDs; i++) {
    pinMode(Out_INFO.LED_pins[i], OUTPUT);
  }
  for (int i = 0; i < tables; i++) {
    pinMode(Out_INFO.table_pins[i], OUTPUT);
  }
  pinMode(In_INFO.input_pin, INPUT);
}

void loop() {
  static char event = 0;

  // there is an event
  if (event = check_Input()) {
    eventHandler(event);

    // keep pressing
    while (check_Input()) {
      lightUp();
    }
  }
  // there is no event
  lightUp();
}

char eventHandler(char event) {  
  static struct Operation operation = {
    0,              // current_stage
    0,              // display_stage
    {0, 0, 0},      // operand[3]
    '?',            // operate
    { -1, -1, -1}   // dot_position[3]
  };
  arithmeticEvent(event, &operation);
  uploadBuffer(&operation);
  return 0;
}

void arithmeticEvent(char event, struct Operation* o) {
  int stage = o-> current_stage;

  o->display_stage = stage;

  // case 1. restart
  if (stage == 2 || event == 'C') {
    for (int i = 0; i < 3; i++) {
      o->operand[i] = 0;
      o->dot_position[i] = -1;
    }
    o-> operate='?';
    o-> current_stage = 0;
    o-> display_stage = 0;

    // case 2. input numbers
  } else if (event >= '0' && event <= '9' && Log(10,o->operand[stage])+o->dot_position[stage]<tables-1) {
    int num = event - '0';
    if (o->dot_position[stage] > -1) {
      o->dot_position[stage]++;
      o->operand[stage] += (double)num / Pow(10, o->dot_position[stage]);
    } else
      o->operand[stage] = (long)o->operand[stage] * 10 + num;

    // case 3. dot
  } else if (event == '.') {
    o->dot_position[stage] = 0;
    
    // case 4. two-number operator
  } else if (event >= '*' && event <= '/' && o->operate=='?') {
    o-> operate = event;
    o-> current_stage = 1;
    
    // case 5. one-number operator
  } else if (event == 'R' && o->operate=='?') {
    o->operand[2] = sqrt(o->operand[0]);
    o-> current_stage = 2;
    o-> display_stage = 2;
    o-> dot_position[2] = tables - 1 - Log(10, o->operand[2]);

    // case 6. equal
  } else if (stage == 1 && event == '=') {
    switch (o->operate) {
      case '+':
        o->operand[2] = o->operand[0] + o->operand[1];
        break;
      case '-':
        o->operand[2] = o->operand[0] - o->operand[1];
        break;
      case '*':
        o->operand[2] = o->operand[0] * o->operand[1];
        break;
      case '/':
        o->operand[2] = o->operand[0] / o->operand[1];
    }
    if((long)o->operand[2]<0) o->operand[2]=0;
    o-> current_stage = 2;
    o-> display_stage = 2;
    o-> dot_position[2] = tables - 1 - Log(10, o->operand[2]);
  }
}

void uploadBuffer(struct Operation* o) {
  int stage = o-> display_stage;

  // case 1. it's integer
  if (o->operand[stage] - (long)o->operand[stage] == 0) {
    for (int i = 0; i < tables; i++) {
      buffer[i] =
        i && o->operand[stage] < Pow(10, i) ?
        20 :
        (long)o->operand[stage] / Pow(10, i) % 10;
    }

    // case 2. too big
  } else if (o->operand[stage] > Pow(10, tables)) {
    for (int i = 0; i < tables; i++)
      buffer[i] =
        (long)o->operand[stage] / Pow(10, i) % 10;

    // case 3. there are points
  } else {
    long integerize = o->operand[stage] * Pow(10, o->dot_position[stage]);
    for (int i = 0; i < tables; i++)
      buffer[i] =
        integerize < Pow(10, i) ?
        20 :
        integerize / Pow(10, i) % 10 + 10 * (o->dot_position[stage] == i);
  }
}

void lightUp() {
  for (int i = 0; i < tables; i++) {
    // light the LED
    for (int j = 0; j < LEDs; j++) {
      digitalWrite(Out_INFO.LED_pins[j],
                   Out_INFO.digital_mapping[buffer[i]][j]);
    }

    // control the table pins
    digitalWrite(Out_INFO.table_pins[i], LOW);

    // keep last light, so that you can sighted-stopped
    delay(2);

    // fastly close last light, so that it won't be influenced by next step
    digitalWrite(Out_INFO.table_pins[i], HIGH);
  }
}

char check_Input() {
  int read_value;
  read_value = analogRead(In_INFO.input_pin);

  // nothing happened, or catch a garbage signal
  if (read_value < In_INFO.min_value)
    return 0;

  // something happened
  for (int i =  0; i < inputs; i++) {
    if (read_value > In_INFO.intervals[i] - 12)
      return In_INFO.events_mapping[(inputs-1)-i];
  }
  return -1;
}

long Log(long base, long x) {
  long i = 0;
  if(base>x) return 0;
  while (Pow(base, i + 1) <= x) {
    i++;
  }
  return i;
}

long Pow(long base , long power) {
  long i = 1;
  while (power--)
    i *= base;
  return i;
  
}
