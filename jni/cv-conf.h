/**
 * @file cv-conf.h
 * @brief Variables of configuration.
 * @author Antonio Manuel Gutierrez Martinez
 * @version 1.0
 */

#ifndef CONF_H
#define CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/** Number of cols of a tag. */
extern int COLS;
/** Number of rows of a tag. */
extern int ROWS;

/** Limit of degrees from which the tag will be deskewed. */
extern int LIMIT_ROTATION;
/** Minimum rectangle size allow in order to consider the contour a rectangle. Size in pixels */
extern int MIN_RECT_SIZE;
/** Width of horizontal border concerning to the tag height. */
extern float WIDTH_BORDER;
/** Width of vertical border concerning to the tag width. */
extern float HEIGHT_BORDER;

/** Debug mode. */
extern bool DEBUG;

#ifdef __cplusplus
}
#endif

#endif //CONF_H