#import "TSUtil.h"

#import <Foundation/Foundation.h>
#import <spawn.h>
#import <sys/sysctl.h>
#import <mach-o/dyld.h>

#define POSIX_SPAWN_PERSONA_FLAGS_OVERRIDE 1
extern int posix_spawnattr_set_persona_np(const posix_spawnattr_t* __restrict, uid_t, uint32_t);
extern int posix_spawnattr_set_persona_uid_np(const posix_spawnattr_t* __restrict, uid_t);
extern int posix_spawnattr_set_persona_gid_np(const posix_spawnattr_t* __restrict, uid_t);

NSString *commandPath(NSString *command)
{
	return [[NSBundle mainBundle].bundlePath stringByAppendingFormat:@"/bin/%@", command];
}

int fd_is_valid(int fd)
{
	return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

NSString* getNSStringFromFile(int fd)
{
	NSMutableString* ms = [NSMutableString new];
	ssize_t num_read;
	char c;
	if(!fd_is_valid(fd)) return @"";
	while((num_read = read(fd, &c, sizeof(c))))
	{
		[ms appendString:[NSString stringWithFormat:@"%c", c]];
		if(c == '\n') break;
	}
	return ms.copy;
}

int spawnRoot(NSString* path, NSArray* args, NSString** stdOut, NSString** stdErr)
{
	NSMutableArray* argsM = args.mutableCopy ?: [NSMutableArray new];
	[argsM insertObject:path atIndex:0];
	
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

	__block volatile BOOL _isRunning = YES;
	NSMutableString* outString = [NSMutableString new];
	NSMutableString* errString = [NSMutableString new];
	dispatch_semaphore_t sema = 0;
	dispatch_queue_t logQueue;
	if(stdOut || stdErr)
	{
		logQueue = dispatch_queue_create("com.opa334.TrollStore.LogCollector", NULL);
		sema = dispatch_semaphore_create(0);

		int outPipe = out[0];
		int outErrPipe = outErr[0];

		__block BOOL outEnabled = (BOOL)stdOut;
		__block BOOL errEnabled = (BOOL)stdErr;
		dispatch_async(logQueue, ^
		{
			while(_isRunning)
			{
				@autoreleasepool
				{
					if(outEnabled)
					{
						[outString appendString:getNSStringFromFile(outPipe)];
					}
					if(errEnabled)
					{
						[errString appendString:getNSStringFromFile(outErrPipe)];
					}
				}
			}
			dispatch_semaphore_signal(sema);
		});
	}

	do
	{
		if (waitpid(task_pid, &status, 0) != -1) {
			NSLog(@"Child status %d", WEXITSTATUS(status));
		} else
		{
			perror("waitpid");
			_isRunning = NO;
			return -222;
		}
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));

	_isRunning = NO;
	if(stdOut || stdErr)
	{
		if(stdOut)
		{
			close(out[1]);
		}
		if(stdErr)
		{
			close(outErr[1]);
		}

		// wait for logging queue to finish
		dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);

		if(stdOut)
		{
			*stdOut = outString.copy;
		}
		if(stdErr)
		{
			*stdErr = errString.copy;
		}
	}

	return WEXITSTATUS(status);
}
