#define HELP_MSG_KEY \
	"Usage: xorator key [options]\n" \
	"Options:\n" \
	"    -f <file>\tspecifies location of input file\n" \
	"    -i <cmd>\tspecifies command to read stdout from\n" \
	"        \tcan't be used with -f\n" \
	"    -o <file>\tspecifies location of output file\n" \
	"    \t\totherwise outputs to stdout\n"

#define HELP_MSG_XOR \
	"Usage: xorator xor [options]\n" \
	"Options:\n" \
	"    -i <icmd>\tspecifies command to read stdout from\n" \
	"    -k <kcmd>\tspecifies command to read key from\n" \
	"    -o <file>\tspecifies location of output file\n" \

#define HELP_MSG \
	"Usage:\n" \
	"    xorator xor [options]\n" \
	"    xorator key [options]\n"

#define TOOMANYARGS -304
int parser(char *src, char **dst, int length)
{
	char *token = strtok(src, " \t\n");
	int iter = 0;

	while (token != NULL) {
		dst[iter] = token;
		if (iter == length)
			return TOOMANYARGS;
		iter++;
		token = strtok(NULL, " \t\n");
	}
	dst[iter] = NULL;
	return iter;
}

#define ARG_NUM 128
int fork_out(char *cmd)
{
	char *args[ARG_NUM];
	int pipes[2];
	pid_t pid;

	if (pipe(pipes) == -1)
		return -1;
	pid = fork();
	if (pid == 0) {
		// Child
		// Closing stdout
		if (close(1) == -1)
			return -1;
		// Replacing closed stdout with pipe output
		if (dup(pipes[1]) == -1)
			return -1;
		if (close(pipes[0]) == -1)
			return -1;
		if (close(pipes[1]) == -1)
			return -1;
		if (parser(cmd, args, ARG_NUM) == TOOMANYARGS)
			return TOOMANYARGS;
		execvp(args[0], args);
		// Error
		exit(EXIT_FAILURE);
	}
	// Parent
	if (pid == -1)
		return -1;
	if (close(pipes[1]) == -1)
		return -1;
	return pipes[0];
}

void key(char *ifile, char *cmd, char *ofile)
{
	int out, in;
	char c;

	if (ofile != NULL) {
		out = creat(ofile, 0666);
		HANDLE_ERROR(out, -1);
	} else
		out = 1;

	if (ifile != NULL) {
		in = open(ifile, O_RDONLY);
		HANDLE_ERROR(in, -1);
	} else {
		in = fork_out(cmd);
		HANDLE_ERROR(in, -1);
		if (in == -2) {
			fprintf(stderr, "Error: two many arguments in cmd\n");
			exit(EXIT_FAILURE);
		}
	}

	srand(time(NULL));
	while (c = read(in, &c, 1)) {
		HANDLE_ERROR(c, -1);
		c = rand()%256;
		write(out, &c, 1);
	}
}

void xor(char *icmd, char *kcmd, char *ofile)
{
	int in, out, key, res_key, res_in;
	char in_byte, out_byte;

	if (ofile != NULL) {
		out = creat(ofile, 0666);
		HANDLE_ERROR(out, -1);
	} else
		out = 1;

	in = fork_out(icmd);
	HANDLE_ERROR(in, -1);
	if (in == -2) {
		fprintf(stderr, "Error: two many arguments in icmd\n");
		exit(EXIT_FAILURE);
	}
	key = fork_out(kcmd);
	HANDLE_ERROR(key, -1);
	if (key == -2) {
		fprintf(stderr, "Error: two many arguments in icmd\n");
		exit(EXIT_FAILURE);
	}

	while ((res_key = read(key, &out_byte, 1)) > 0
		&& (res_in = read(in, &in_byte, 1)) > 0) {
		out_byte ^= in_byte;
		HANDLE_ERROR(write(out, &out_byte, 1), -1);
	}
	HANDLE_ERROR(res_in, -1);
	HANDLE_ERROR(res_key, -1);
}

int main(int argn, char **argv)
{
	int opt;		// Result from getopt
	char f_wrong;	// Flag-validator of input args
	char *ifile, *icmd, *ofile, *kcmd;

	f_wrong = opt = 0;
	if (argn > 2 && !strcmp(argv[1], "key")) {
		ifile = icmd = ofile = NULL;
		while ((opt = getopt(argn, argv, "f:i:o:")) != -1) {
			switch (opt) {
			case 'i':
				icmd = optarg;
				break;
			case 'f':
				ifile = optarg;
				break;
			case 'o':
				ofile = optarg;
				break;
			case '?':
				f_wrong = 1;
			}
		}
		// Errors:
		// getopt() returns '?'
		// both ifile and icmd provided
		// neither ifile nor icmd provided
		if (f_wrong || ifile == icmd
			|| ifile != NULL && icmd != NULL) {
			printf(HELP_MSG_KEY);
			return -1;
		}
		key(ifile, icmd, ofile);
	} else if (argn > 2 && !strcmp(argv[1], "xor")) {
		icmd = kcmd = ofile = NULL;
		while ((opt = getopt(argn, argv, "i:k:o:")) != -1) {
			switch (opt) {
			case 'i':
				icmd = optarg;
				break;
			case 'k':
				kcmd = optarg;
				break;
			case 'o':
				ofile = optarg;
				break;
			case '?':
				f_wrong = 1;
			}
		}
		if (f_wrong || icmd == NULL || kcmd == NULL) {
			printf(HELP_MSG_XOR);
			return -1;
		}
		xor(icmd, kcmd, ofile);
	} else
		printf(HELP_MSG);
	return 0;
}