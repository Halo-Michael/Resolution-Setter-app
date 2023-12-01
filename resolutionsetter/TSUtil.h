#import <Foundation/Foundation.h>

NSString *commandPath(NSString *command);
int spawnRoot(NSString* path, NSArray* args, NSString** stdOut, NSString** stdErr);
