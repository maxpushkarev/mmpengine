#import <Backend/Metal/Window.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

namespace MMPEngine::Backend::Metal
{
    void Window::JoinMetalLayerToWindow(void* nativeWindow, CA::MetalLayer* metalLayer)
    {

        NSWindow* nsWindow = static_cast<NSWindow*>(nativeWindow);
        NSView* contentView = [nsWindow contentView];

        CAMetalLayer* ml = (CAMetalLayer*)metalLayer;
        [contentView setWantsLayer:YES];
        [contentView setLayer:ml];
    }
}
