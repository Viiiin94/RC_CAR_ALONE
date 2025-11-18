#ifndef INC_MODE_H_
#define INC_MODE_H_

typedef enum {
	MODE_JOYSTICK = 0,
	MODE_AUTOMOBILITY = 1
} Carmode;

extern Carmode current_mode;

void mode_init(void);
void mode_switch(void);
Carmode mode_get(void);
void mode_set(Carmode new_mode);

#endif /* INC_MODE_H_ */
