//
//  ViewController.h
//  resolutionsetter
//
//  Created by apple on 2022/10/7.
//

#import <UIKit/UIKit.h>
#import <removefile.h>
#import "helpers.h"

#define PROC_ALL_PIDS        1
#define PROC_PIDPATHINFO_MAXSIZE    (4*MAXPATHLEN)
#define SafeFree(x) do { if (x) free(x); } while(false)
#define SafeFreeNULL(x) do { SafeFree(x); (x) = NULL; } while(false)

int proc_listpids(uint32_t type, uint32_t typeinfo, void *buffer, int buffersize);
int proc_pidpath(int pid, void *buffer, uint32_t buffersize);

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

char *get_path_for_pid(pid_t pid) {
	char *ret = NULL;
	uint32_t path_size = PROC_PIDPATHINFO_MAXSIZE;
	char *path = malloc(path_size);
	if (path != NULL) {
		if (proc_pidpath(pid, path, path_size) >= 0)
			ret = strdup(path);
		SafeFreeNULL(path);
	}
	return ret;
}

pid_t pidOfProcess(const char *name) {
	char real[PROC_PIDPATHINFO_MAXSIZE];
	bzero(real, sizeof(real));
	realpath(name, real);
	int numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
	pid_t pids[numberOfProcesses];
	bzero(pids, sizeof(pids));
	proc_listpids(PROC_ALL_PIDS, 0, pids, (int)sizeof(pids));
	bool foundProcess = false;
	pid_t processPid = 0;
	for (int i = 0; i < numberOfProcesses && !foundProcess; ++i) {
		if (pids[i] == 0)
			continue;
		char *path = get_path_for_pid(pids[i]);
		if (path != NULL) {
			if (strlen(path) > 0 && strcmp(path, real) == 0) {
				processPid = pids[i];
				foundProcess = true;
			}
			SafeFreeNULL(path);
		}
	}
	return processPid;
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
	xpc_crasher("com.apple.cfprefsd.daemon");
	return nil;
}
