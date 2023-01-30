//
//  ViewController.h
//  resolutionsetter
//
//  Created by apple on 2022/10/7.
//

#import <UIKit/UIKit.h>
#import <removefile.h>
#import "helpers.h"

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
