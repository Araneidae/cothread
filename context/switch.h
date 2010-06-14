/* Interface for stack switching. */

/* A saved stack frame is completely defined by a pointer to the top of the
 * stack frame. */
typedef void *frame_t;

typedef __attribute__((noreturn))
    void (*frame_action_t)(void *arg, void *context);

/* Switch to new frame, previously established by create_frame() or an earlier
 * switch_frame().  The frame context is updated. */
void * switch_frame(frame_t *old_frame, frame_t new_frame, void *arg);

/* Retrieves the current frame.  Note that the returned result will be invalid
 * as soon as control is returned, so this is only useful for retrieving an
 * indication of the current high water mark in the stack. */
frame_t get_frame(void);

/* Establish a new frame in the given stack.  action(context) will be called
 * when the newly created frame is switched to.  When the action routine returns
 * control is switched to parent. */
void create_frame(
    frame_t *frame, void *stack_base, frame_action_t action, void *context);
