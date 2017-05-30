//
// Taken from Robert Harder's page
//

#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

#define error(...) fprintf(stderr, __VA_ARGS__)
#define console(...) (!g_quiet && printf(__VA_ARGS__))
#define verbose(...) (g_verbose && !g_quiet && fprintf(stderr, __VA_ARGS__))

BOOL g_verbose = NO;
BOOL g_quiet = NO;

@interface ImageSnap : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate> {
    
    AVCaptureSession                    *mCaptureSession;
    AVCaptureDeviceInput                *mCaptureDeviceInput;
    AVCaptureVideoDataOutput            *mCaptureDecompressedVideoOutput;
    CVImageBufferRef                    mCurrentImageBuffer;
}

/**
 * Returns all attached AVCaptureDevice objects that have video.
 * This includes video-only devices (AVMediaTypeVideo) and
 * audio/video devices (AVMediaTypeMuxed).
 *
 * @return autoreleased array of video devices
 */
+(NSArray *)videoDevices;

/**
 * Returns the default AVCaptureDevice object for video
 * or nil if none is found.
 */
+(AVCaptureDevice *)defaultVideoDevice;

/**
 * Returns the AVCaptureDevice with the given name
 * or nil if the device cannot be found.
 */
+(AVCaptureDevice *)deviceNamed:(NSString *)name;

/**
 * Writes an NSImage to disk, formatting it according
 * to the file extension. If path is "-" (a dash), then
 * an jpeg representation is written to standard out.
 */
+ (BOOL) saveImage:(NSImage *)image toPath: (NSString*)path;

/**
 * Converts an NSImage to raw NSData according to a given
 * format. A simple string search is performed for such
 * characters as jpeg, tiff, png, and so forth.
 */
+(NSData *)dataFrom:(NSImage *)image asType:(NSString *)format;

-(id)init;
-(void)dealloc;

-(BOOL)startSession:(AVCaptureDevice *)device withWidth:(unsigned int)width withHeight:(unsigned int)height;
-(CIImage *)snapshot;
-(void)stopSession;

@end
