//
//  ViewController.h
//  resolutionsetter
//
//  Created by apple on 2022/10/7.
//

#import <UIKit/UIKit.h>
#import <removefile.h>
#import <spawn.h>

#define POSIX_SPAWN_PERSONA_FLAGS_OVERRIDE 1
extern int posix_spawnattr_set_persona_np(const posix_spawnattr_t* __restrict, uid_t, uint32_t);
extern int posix_spawnattr_set_persona_uid_np(const posix_spawnattr_t* __restrict, uid_t);
extern int posix_spawnattr_set_persona_gid_np(const posix_spawnattr_t* __restrict, uid_t);

#define bundleID CFSTR("com.apple.iokit.IOMobileGraphicsFamily")
#define userName CFSTR("mobile")

@interface ViewController : UIViewController

@property (weak, nonatomic) IBOutlet UITextField *Height;
@property (weak, nonatomic) IBOutlet UITextField *Width;
@property (weak, nonatomic) IBOutlet UIButton *Set;

@end

BOOL vaildNumber(NSString *string) {
	NSScanner *scan = [NSScanner scannerWithString:string];
	int val;
	if ([scan scanInt:&val] && [scan isAtEnd]) {
		NSNumber *number = @([string longLongValue]);
		if (([number compare:@99] == NSOrderedDescending) && ([number compare:@10000] == NSOrderedAscending))
			return YES;
	}
	return NO;
}

NSString* commandPath(NSString* command)
{
	return [[NSBundle mainBundle].bundlePath stringByAppendingFormat:@"/bin/%@", command];
}

NSString* getNSStringFromFile(int fd)
{
	NSMutableString* ms = [NSMutableString new];
	ssize_t num_read;
	char c;
	while((num_read = read(fd, &c, sizeof(c))))
	{
		[ms appendString:[NSString stringWithFormat:@"%c", c]];
	}
	return ms.copy;
}

int spawnRoot(NSString* path, NSArray* args, NSString** stdOut, NSString** stdErr)
{
	NSMutableArray* argsM = args.mutableCopy ?: [NSMutableArray new];
	[argsM insertObject:path.lastPathComponent atIndex:0];
	
	NSUInteger argCount = [argsM count];
	char **argsC = (char **)malloc((argCount + 1) * sizeof(char*));

	for (NSUInteger i = 0; i < argCount; i++)
	{
		argsC[i] = strdup([[argsM objectAtIndex:i] UTF8String]);
	}
	argsC[argCount] = NULL;

	posix_spawnattr_t attr;
	posix_spawnattr_init(&attr);

	posix_spawnattr_set_persona_np(&attr, 99, POSIX_SPAWN_PERSONA_FLAGS_OVERRIDE);
	posix_spawnattr_set_persona_uid_np(&attr, 0);
	posix_spawnattr_set_persona_gid_np(&attr, 0);

	posix_spawn_file_actions_t action;
	posix_spawn_file_actions_init(&action);

	int outErr[2];
	if(stdErr)
	{
		pipe(outErr);
		posix_spawn_file_actions_adddup2(&action, outErr[1], STDERR_FILENO);
		posix_spawn_file_actions_addclose(&action, outErr[0]);
	}

	int out[2];
	if(stdOut)
	{
		pipe(out);
		posix_spawn_file_actions_adddup2(&action, out[1], STDOUT_FILENO);
		posix_spawn_file_actions_addclose(&action, out[0]);
	}
	
	pid_t task_pid;
	int status = -200;
	int spawnError = posix_spawn(&task_pid, [path UTF8String], &action, &attr, (char* const*)argsC, NULL);
	posix_spawnattr_destroy(&attr);
	for (NSUInteger i = 0; i < argCount; i++)
	{
		free(argsC[i]);
	}
	free(argsC);
	
	if(spawnError != 0)
	{
		NSLog(@"posix_spawn error %d\n", spawnError);
		return spawnError;
	}

	do
	{
		if (waitpid(task_pid, &status, 0) != -1) {
			NSLog(@"Child status %d", WEXITSTATUS(status));
		} else
		{
			perror("waitpid");
			return -222;
		}
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));

	if(stdOut)
	{
		close(out[1]);
		NSString* output = getNSStringFromFile(out[0]);
		*stdOut = output;
	}

	if(stdErr)
	{
		close(outErr[1]);
		NSString* errorOutput = getNSStringFromFile(outErr[0]);
		*stdErr = errorOutput;
	}
	
	return WEXITSTATUS(status);
}

NSDictionary *loadPrefs() {
	CFArrayRef keyList = CFPreferencesCopyKeyList(bundleID, userName, kCFPreferencesAnyHost);
	if (keyList != NULL) {
		CFDictionaryRef cfPrefs = CFPreferencesCopyMultiple(keyList, bundleID, userName, kCFPreferencesAnyHost);
		CFRelease(keyList);
		NSDictionary *prefs = CFBridgingRelease(cfPrefs);
		if (prefs[@"canvas_height"] != nil && [prefs[@"canvas_height"] isKindOfClass:[NSNumber class]] && ([prefs[@"canvas_height"] compare:@99] == NSOrderedDescending) && [prefs[@"canvas_height"] compare:@10000] == NSOrderedAscending && prefs[@"canvas_width"] != nil && [prefs[@"canvas_width"] isKindOfClass:[NSNumber class]] && ([prefs[@"canvas_width"] compare:@99] == NSOrderedDescending) && [prefs[@"canvas_width"] compare:@10000] == NSOrderedAscending)
			return prefs;
	}
	removefile("/private/var/mobile/Library/Preferences/com.apple.iokit.IOMobileGraphicsFamily.plist", NULL, REMOVEFILE_RECURSIVE);
	//CFPreferencesSynchronize(bundleID, userName, kCFPreferencesAnyHost);
	spawnRoot([[NSBundle mainBundle].bundlePath stringByAppendingPathComponent:@"killall"], @[@"-9", @"cfprefsd"], nil, nil);
	return nil;
}
