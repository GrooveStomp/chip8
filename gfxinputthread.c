/******************************************************************************
  File: gfxinputthread.c
  Created: 2019-07-25
  Updated: 2019-07-27
  Author: Aaron Oman
  Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 ******************************************************************************/
// NOTE: Kinda dangerous - anybody in this translation unit can access ui
// without a synchronization primitive.  This may necessitate putting this c
// file into a completely separate translation unit.
static struct ui *ui;

void UIRenderFn() {
        UIRender(ui);
}

void *gfxInputWork(void *context) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wpointer-arith"
        struct thread_args *ctx = (struct thread_args *)context;
        #pragma GCC diagnostic pop

        static const double frequency = 1 / 30; // 30 FPS in MS per frame.

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
                /* struct timespec start; */
                /* clock_gettime(CLOCK_REALTIME, &start); */

                UIInputBegin(ui);
                while (SDL_PollEvent(&event)) {
                        InputCheck(input, ctx->sys, &event);
                        UIHandleEvent(ui, &event);
                }
                UIInputEnd(ui);

                UIWidgets(ui, ctx->sys, ctx->opcode);
                GraphicsPresent(graphics, ctx->sys, UIRenderFn);

                /* struct timespec end; */
                /* clock_gettime(CLOCK_REALTIME, &end); */

                /* double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms */
                /* elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000.0; // us to ms */

                /* struct timespec sleep = { .tv_sec = 0, .tv_nsec = (frequency - elapsed_time) * 1000 }; */
                /* nanosleep(&sleep, NULL); */
        }

        UIDeinit(ui);
        InputDeinit(input);
        GraphicsDeinit(graphics);

        return NULL;
}
