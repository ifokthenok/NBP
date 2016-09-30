#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <cutils/log.h>
#include <wayland-client.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "wayland-client"


#define COORDINATE_FACTOR   		256
#define KEY_LEFT            		272 
#define KEY_RIGHT           		273 
#define KEY_MID             		274 
#define KEY_STATE_UP        		0 
#define KEY_STATE_DOWN      		1 

#define ANDROID_KEY_HOME    		3
#define ANDROID_KEY_BACK    		4

#define MOTION_GAP_THRESHOLD        20 		// in pixels
#define BUTTON_CLICK_GAP_THRESHOLD  1000 	// 1000 milliseconds to determine R/L button are clicked at same time
#define MAX_CMD_LEN                 50

#define INPUT_SOURCE_MOUSE          "mouse"
#define INPUT_SOURCE_KEY            "keyevent"
#define INPUT_SOURCE_TOUCH          "touchscreen"

#define TOUCH_ACTION_DOWN           "touchdown"
#define TOUCH_ACTION_UP             "touchup"
#define TOUCH_ACTION_MOVE           "touchmove"

#define MAX_MULTIPLE_TOUCH_POINT_NUM   15   // 15 multiple points are supported in max
#define EVENT_OVERDUE_TIME             3000 //milliseconds, 

#define TOUCH_STATE_UP                 0   
#define TOUCH_STATE_DOWN               1

typedef enum  {
   touch_event_up = 0,
   touch_event_down,
   touch_event_motion,
   touch_event_frame,
   touch_event_cancel,
   touch_event_max
} TouchEventType;

typedef struct  {
    struct TouchDownData {
        int32_t   id;
        wl_fixed_t x; 
        wl_fixed_t y; 
        uint32_t   time;
    } lastDown;

    wl_fixed_t  lastTouchX; //updated by touch_down/motion events
    wl_fixed_t  lastTouchY; //updated by touch_down/motion events
    int         lastEventST; // The timestamp of the last event, should be signed integer
    unsigned char state;     // State machine: TOUCH_STATE_UP(touch_down)->TOUCH_STATE_DOWN(touch_up)->TOUCH_STATE_UP
} TouchState;

struct InputState {
   // mouse 
   struct Button {
       uint32_t  button;
       uint32_t  state;
       wl_fixed_t  x;
       wl_fixed_t  y;
       uint32_t    time;
   } lastButton;

   struct Position {
       wl_fixed_t  x;
       wl_fixed_t  y;
   } lastPos;

   bool    enteredSurface;  

   // touch
   // At most 15 simultaneous touch points support, indexed by their touch id
   TouchState touchEvents[MAX_MULTIPLE_TOUCH_POINT_NUM];  

} inputState;

// return: true - It's a valid event
//           false - It's an invalid or duplicate event
bool updateTouchState(TouchEventType eventType, int32_t id, wl_fixed_t x, wl_fixed_t y, int timestamp) {
    if (id >= MAX_MULTIPLE_TOUCH_POINT_NUM ) {
        ALOGW("Event id exceeds the limit of multiple pointer number:%d, id=%d,eventType=%d,x=%d,y=%d,timestamp=%d",
            MAX_MULTIPLE_TOUCH_POINT_NUM,id,eventType,x,y,timestamp);
        return false;
    }
    if (eventType >= touch_event_max ) {        
        ALOGW("Invalid touch event type: id=%d,eventType=%d,x=%d,y=%d,timestamp=%d",
            id,eventType,x,y,timestamp);
        return false; 
    }

    if (touch_event_up == eventType) {
        if (timestamp-inputState.touchEvents[id].lastEventST > EVENT_OVERDUE_TIME ||
            TOUCH_STATE_DOWN == inputState.touchEvents[id].state ) {

            // Update the state
            inputState.touchEvents[id].lastEventST = timestamp;
            inputState.touchEvents[id].state = TOUCH_STATE_UP;            

            //New event
            return true;
        } else {
            ALOGD("Invalid event: id=%d,eventType=%d,x=%d,y=%d,timestamp=%d",
                id,eventType,x,y,timestamp);
            return false;
        }            
    } else if (touch_event_down == eventType) {
        if ( timestamp-inputState.touchEvents[id].lastEventST > EVENT_OVERDUE_TIME ||
            TOUCH_STATE_UP == inputState.touchEvents[id].state) {

            // Update the state
            inputState.touchEvents[id].lastDown.x = x;
            inputState.touchEvents[id].lastDown.y = y;
            inputState.touchEvents[id].lastDown.id = id;
            inputState.touchEvents[id].lastDown.time = timestamp;

            inputState.touchEvents[id].lastTouchX = x;
            inputState.touchEvents[id].lastTouchY = y;
            inputState.touchEvents[id].lastEventST = timestamp;
            inputState.touchEvents[id].state = TOUCH_STATE_DOWN;            
            // New event
            return true;        
        } else {
            ALOGD("Invalid event: id=%d,eventType=%d,x=%d,y=%d,timestamp=%d",
                id,eventType,x,y,timestamp);
            return false;
        }
    }  else if (touch_event_motion== eventType) {   
        if ( timestamp-inputState.touchEvents[id].lastEventST > EVENT_OVERDUE_TIME ||
            (TOUCH_STATE_DOWN == inputState.touchEvents[id].state &&
            ( x != inputState.touchEvents[id].lastTouchX ||
              y != inputState.touchEvents[id].lastTouchY))) {
            
            // update the state
            inputState.touchEvents[id].lastTouchX = x;
            inputState.touchEvents[id].lastTouchY = y;
            inputState.touchEvents[id].lastEventST = timestamp;            
            // New event
            return true;
        } else {
            ALOGD("Invalid event: id=%d,eventType=%d,x=%d,y=%d,timestamp=%d",
                id,eventType,x,y,timestamp);
            return false;
        }
        
    }

    ALOGD("Invalid event: id=%d,eventType=%d,x=%d,y=%d,timestamp=%d",
            id,eventType,x,y,timestamp);
    return false;
}

void injectInputKey(uint32_t  key) {

    ALOGD("injectInputKey(): key=%d", key);
    // output the command to the stdout, and piped into input's stdin
    printf("%s %d\n",INPUT_SOURCE_KEY, key);
    fflush(stdout);
}
void injectInputTap(char* source, uint32_t  x, uint32_t y) {

    ALOGD("injectInputTap(): source=%s,x=%d, y=%d",source,x,y);
    // output the command to the stdout, and piped into input's stdin
    printf("%s tap %d %d\n", source,x,y);
    fflush(stdout);
}

void injectInputTouchEvent(char* action, uint32_t  x, uint32_t y) {

    ALOGD("injectInputTouchEvent(): action=%s,x=%d, y=%d",action,x,y);
    // output the command to the stdout, and piped into input's stdin
    printf("%s %s %d %d\n", INPUT_SOURCE_TOUCH, action,x,y);
    fflush(stdout);
}

void injectInputSwipe(char* source, uint32_t x1,uint32_t y1, uint32_t x2, uint32_t y2, uint32_t duration) {

    duration = 30;

    ALOGD("injectInputSwipe(): source=%s,%d %d %d %d %d\n",source,x1,y1,x2,y2,duration);
    // output the command to the stdout, and piped into input's stdin
    printf("%s swipe %d %d %d %d %d\n",source,x1,y1,x2,y2,duration);
    fflush(stdout);
}

static void pointer_enter(void *data, 
	struct wl_pointer *wl_pointer,
    uint32_t serial, struct wl_surface *surface,
    wl_fixed_t surface_x, wl_fixed_t surface_y) {

	ALOGD("pointer_enter()");
	// Commented out the following code line as it hides the cursor on alpine/TI platform
	// but however works on conti platform:
	//wl_pointer_set_cursor(wl_pointer, serial, NULL, 1,  1);

    inputState.enteredSurface = true;
}

static void pointer_leave(void* data,
    struct wl_pointer* wl_pointer, uint32_t serial,
    struct wl_surface* wl_surface) { 
    ALOGD("pointer_leave()");

    inputState.enteredSurface = false;
}

static void pointer_motion(void* data,
    struct wl_pointer* wl_pointer, uint32_t time,
    wl_fixed_t surface_x, wl_fixed_t surface_y) { 

    surface_x = (double)surface_x/(double)COORDINATE_FACTOR;
    surface_y = (double)surface_y/(double)COORDINATE_FACTOR;

    inputState.lastPos.x = surface_x; 
    inputState.lastPos.y = surface_y ;
}

static void pointer_button(void* data,
    struct wl_pointer* wl_pointer, uint32_t serial,
    uint32_t time, uint32_t button, uint32_t state) {

	ALOGD("pointer_button(): serial=%d, button=%d, state=%d,time=%d", 
		serial, button, state,time);

    if (inputState.enteredSurface) { 
        if ( KEY_LEFT == button &&  KEY_STATE_UP == state && 
             KEY_LEFT == inputState.lastButton.button && KEY_STATE_DOWN == inputState.lastButton.state  ) {
    
            if (abs(inputState.lastButton.x-inputState.lastPos.x) >= MOTION_GAP_THRESHOLD ||
                abs(inputState.lastButton.y-inputState.lastPos.y) >= MOTION_GAP_THRESHOLD ) { 
                //It's swipe
                injectInputSwipe(INPUT_SOURCE_MOUSE,inputState.lastButton.x,inputState.lastButton.y,
                    inputState.lastPos.x, inputState.lastPos.y, abs(time-inputState.lastButton.time));
            } else { 
                // it's tap
                injectInputTap(INPUT_SOURCE_MOUSE,inputState.lastPos.x, inputState.lastPos.y);
            }
        }  else if ((KEY_MID == button && KEY_STATE_DOWN == state)
             ||
             ( KEY_STATE_DOWN == state && KEY_STATE_DOWN == inputState.lastButton.state &&
               KEY_LEFT == inputState.lastButton.button && KEY_RIGHT == button &&
               abs(time-inputState.lastButton.time) <  BUTTON_CLICK_GAP_THRESHOLD)) {
            // LEFT->RIGHT buttons are clicked in turn, or middle button is clicked.           
            injectInputKey(ANDROID_KEY_BACK);
        }
    }

    inputState.lastButton.button = button;
    inputState.lastButton.state = state;
    inputState.lastButton.time = time;
    inputState.lastButton.x= inputState.lastPos.x;
    inputState.lastButton.y= inputState.lastPos.y;
}

static void pointer_axis(void* data,
    struct wl_pointer* wl_pointer, uint32_t time,
    uint32_t axis, wl_fixed_t value) { 
    ALOGD("pointer_axis(): axis=%d, value=%d", axis, value);
}

const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis
};

/////////////////////////////////////////////////////////////////
//
// Touch listen
//
/////////////////////////////////////////////////////////////////
/**
 * down - touch down event and beginning of a touch sequence
 * @serial: (none)
 * @time: timestamp with millisecond granularity
 * @surface: (none)
 * @id: the unique ID of this touch point
 * @x: x coordinate in surface-relative coordinates
 * @y: y coordinate in surface-relative coordinates
 *
 * A new touch point has appeared on the surface. This touch
 * point is assigned a unique @id. Future events from this
 * touchpoint reference this ID. The ID ceases to be valid after a
 * touch up event and may be re-used in the future.
 */
void touch_down(void *data,
	     struct wl_touch *wl_touch,
	     uint32_t serial,
	     uint32_t time,
	     struct wl_surface *surface,
	     int32_t id,
	     wl_fixed_t x,
	     wl_fixed_t y) {

    ALOGD("touch_down(): id=%d,x=%d, y=%d", id,x, y);
    x = (double)x/(double)COORDINATE_FACTOR;
    y = (double)y/(double)COORDINATE_FACTOR;    
    ALOGD("converted: x=%d, y=%d", x, y);
    if (!updateTouchState(touch_event_down, id, x, y,time)) {
        return;
    }

    injectInputTouchEvent(TOUCH_ACTION_DOWN,x,y);
}

/**
 * up - end of a touch event sequence
 * @serial: (none)
 * @time: timestamp with millisecond granularity
 * @id: the unique ID of this touch point
 *
 * The touch point has disappeared. No further events will be
 * sent for this touchpoint and the touch point's ID is released
 * and may be re-used in a future touch down event.
 */
void touch_up(void *data,
	   struct wl_touch *wl_touch,
	   uint32_t serial,
	   uint32_t time,
	   int32_t id) {

    ALOGD("touch_up(): id=%d", id);

    if (!updateTouchState(touch_event_up, id, 0, 0,time)) {
        return;
    }
#if 0
    TouchState * eventp = & inputState.touchEvents[id];
    
    if (abs(eventp->lastDown.x - eventp->lastTouchX) >= MOTION_GAP_THRESHOLD || 
    abs(eventp->lastDown.y - eventp->lastTouchY) >= MOTION_GAP_THRESHOLD )  {
        // It's swip        
        injectInputSwipe(INPUT_SOURCE_TOUCH,eventp->lastDown.x,eventp->lastDown.y,
           eventp->lastTouchX, eventp->lastTouchY, abs(time - eventp->lastDown.time));
    } else {
        // It's tap
        injectInputTap(INPUT_SOURCE_TOUCH,eventp->lastDown.x, eventp->lastDown.y);
    }
#else
    injectInputTouchEvent(TOUCH_ACTION_UP,inputState.touchEvents[id].lastTouchX,inputState.touchEvents[id].lastTouchY);
#endif
    

}
/**
 * motion - update of touch point coordinates
 * @time: timestamp with millisecond granularity
 * @id: the unique ID of this touch point
 * @x: x coordinate in surface-relative coordinates
 * @y: y coordinate in surface-relative coordinates
 *
 * A touchpoint has changed coordinates.
 */
void touch_motion(void *data,
	       struct wl_touch *wl_touch,
	       uint32_t time,
	       int32_t id,
	       wl_fixed_t x,
	       wl_fixed_t y) {
	       
    ALOGD("touch_motion(): id=%d,x=%d, y=%d", id,x, y);
    x = (double)x/(double)COORDINATE_FACTOR;
    y = (double)y/(double)COORDINATE_FACTOR;
    ALOGD("converted: x=%d, y=%d", x, y);

    if (!updateTouchState(touch_event_motion, id, x, y,time)) {
        return;
    }

    injectInputTouchEvent(TOUCH_ACTION_MOVE,x,y);
}
/**
 * frame - end of touch frame event
 *
 * Indicates the end of a contact point list.
 */
void touch_frame(void *data,
	      struct wl_touch *wl_touch) {
    ALOGD("touch_frame()");	      
}
/**
 * cancel - touch session cancelled
 *
 * Sent if the compositor decides the touch stream is a global
 * gesture. No further events are sent to the clients from that
 * particular gesture. Touch cancellation applies to all touch
 * points currently active on this client's surface. The client is
 * responsible for finalizing the touch points, future touch points
 * on this surface may re-use the touch point ID.
 */
void touch_cancel(void *data,
	       struct wl_touch *wl_touch) {
    ALOGD("touch_cancel()");	       
}           

const struct wl_touch_listener  touch_listener = {
   .down = touch_down,
   .up = touch_up,
   .motion = touch_motion,
   .frame = touch_frame,
   .cancel = touch_cancel
};



