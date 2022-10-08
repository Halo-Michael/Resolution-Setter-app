//
//  ViewController.m
//  resolutionsetter
//
//  Created by apple on 2022/10/7.
//

#import <Foundation/NSUserDefaults.h>
#import "ViewController.h"
#import <sys/stat.h>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
	NSDictionary *prefs = loadPrefs();
	if (prefs != nil) {
		[_Height setText:[NSString stringWithFormat:@"%@", prefs[@"canvas_height"]]];
		[_Width setText:[NSString stringWithFormat:@"%@", prefs[@"canvas_width"]]];
	}
	[super viewDidLoad];
	// Do any additional setup after loading the view.
}

- (IBAction)Set:(id)sender {
	if (vaildNumber(_Height.text) && vaildNumber(_Width.text)) {
		removefile("/private/var/mobile/Library/Preferences/com.apple.iokit.IOMobileGraphicsFamily.plist", NULL, REMOVEFILE_RECURSIVE);
		
		removefile("/private/var/tmp/com.michael.iokit.IOMobileGraphicsFamily", NULL, REMOVEFILE_RECURSIVE);
		mkdir("/private/var/tmp/com.michael.iokit.IOMobileGraphicsFamily", 755);
		lchown("/private/var/tmp/com.michael.iokit.IOMobileGraphicsFamily", 501, 501);
		symlink("../../../tmp/com.michael.iokit.IOMobileGraphicsFamily/com.apple.iokit.IOMobileGraphicsFamily.plist", "/private/var/mobile/Library/Preferences/com.apple.iokit.IOMobileGraphicsFamily.plist");
		lchown("/private/var/mobile/Library/Preferences/com.apple.iokit.IOMobileGraphicsFamily.plist", 501, 501);
		
		NSDictionary *IOMobileGraphicsFamily = [NSDictionary dictionaryWithObjects:@[@([_Height.text longLongValue]), @([_Width.text longLongValue])] forKeys:@[@"canvas_height", @"canvas_width"]];
			[IOMobileGraphicsFamily writeToFile:@"/private/var/tmp/com.michael.iokit.IOMobileGraphicsFamily/com.apple.iokit.IOMobileGraphicsFamily.plist" atomically:NO];
		lchown("/private/var/tmp/com.michael.iokit.IOMobileGraphicsFamily/com.apple.iokit.IOMobileGraphicsFamily.plist", 501, 501);
		//CFPreferencesSynchronize(bundleID, userName, kCFPreferencesAnyHost);
		spawnRoot([[NSBundle mainBundle].bundlePath stringByAppendingPathComponent:@"killall"], @[@"-9", @"cfprefsd"], nil, nil);
		//CFPreferencesSynchronize(bundleID, userName, kCFPreferencesAnyHost);
		sleep(1);
		spawnRoot([[NSBundle mainBundle].bundlePath stringByAppendingPathComponent:@"killall"], @[@"-9", @"backboardd"], nil, nil);
		//killall(@"backboardd");
		//sbreload();
	}
}

@end
