/******************************************************************************
  File: gfxinputthread.c
  Created: 2019-07-25
  Updated: 2019-07-31
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/

//! \file gfxinputthread.c

// NOTE: Kinda dangerous - anybody in this translation unit can access ui
// without a synchronization primitive.  This may necessitate putting this c
// file into a completely separate translation unit.
static struct ui *ui;

//! \brief A convenience function used in GraphicsPresent()
//!
//! Wraps UIRender() to match the prototype function pointer in GraphicsPresent()
void UIRenderFn() {
        UIRender(ui);
}

//! \brief Thread for graphics and input updates
//!
//! Graphics and input are coupled together on the same thread because I
//! figure both deal with human perception, so their frequency can be similar.
//! Right now this thread is configured to run at 30hz.
//!
//! Decoupling graphics and input allows the emulation engine to run at a
//! much higher frequency and not be limited by drawing routines.
//!
//! Input is included here due to coupling between UI and input.
//!
//! \param[in] context struct thread_args casted to void*
//! \return NULL
void *GFXInputThread(void *context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpointer-arith"
        struct thread_args *ctx = (struct thread_args *)context;
        #pragma GCC diagnostic pop

        static const double msPerFrame = HZ_TO_MS(30);

        struct graphics *graphics = GraphicsInit(ctx->isDebugEnabled);
        if (graphics == NULL) {
                fprintf(stderr, "Couldn't initialize graphics\n");
                return NULL;
        }

        struct input *input = InputInit();
        if (NULL == input) {
                fprintf(stderr, "Couldn't initialize input");
                GraphicsDeinit(graphics);
                return NULL;
        }

        ui = UIInit(ctx->isDebugEnabled, 240, 240, GraphicsSDLWindow(graphics));
        if (NULL == ui) {
                fprintf(stderr, "Couldn't initialize ui\n");
                InputDeinit(input);
                GraphicsDeinit(graphics);
                return NULL;
        }

        SDL_Event event;
        while (!ThreadSyncShouldShutdown(ctx->threadSync)) {
                struct timespec start;
                clock_gettime(CLOCK_REALTIME, &start);

                UIInputBegin(ui);
                while (SDL_PollEvent(&event)) {
                        InputCheck(input, ctx->sys, &event);
                        UIHandleEvent(ui, &event);
                }
                UIInputEnd(ui);

                UIWidgets(ui, ctx->sys, ctx->opcode);
                GraphicsPresent(graphics, ctx->sys, UIRenderFn);

                struct timespec end;
                clock_gettime(CLOCK_REALTIME, &end);

                double elapsedTime = S_TO_MS(end.tv_sec - start.tv_sec);
                elapsedTime += NS_TO_MS(end.tv_nsec - start.tv_nsec);

                struct timespec sleep = { .tv_sec = 0, .tv_nsec = MS_TO_NS(msPerFrame - elapsedTime) };
                nanosleep(&sleep, NULL);
        }

        UIDeinit(ui);
        InputDeinit(input);
        GraphicsDeinit(graphics);

        return NULL;
}
