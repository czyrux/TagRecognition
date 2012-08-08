#ifndef CONF_H
#define CONF_H

#ifdef __cplusplus
extern "C" {
#endif

extern int COLS;
extern int ROWS;


extern int LIMIT_ROTATION; //degrees
extern int MIN_RECT_SIZE; //size in pixels
extern int WIDTH_BORDER;
extern int HEIGHT_BORDER;

extern bool DEBUG;

#ifdef __cplusplus
}
#endif

#endif //CONF_H