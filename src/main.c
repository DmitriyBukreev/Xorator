#define NAME_LEN 256
#define BUF_SIZE 512
#define HELP_MSG \
	"Usage:\n" \
	"xorator -f file_1 [file_2 [...]\n" \
	"\t- generates key for a file named file.xored\n" \
	"xorator -i \"in_cmd\" -k \"key_cmd\" -o file_dst\n" \
	"\t- encrypts/decrypts stdout of in_cmd with a key\n" \
	"\tprovided by stdout of key_cmd and outputs it to file_dst\n"

void xorate(int in, int key, int out)
{

}

void generate(int in, int out)
{
	char c;

	while (c = read(in, &c, 1)) {
		HANDLE_ERROR(c, -1);
		c = rand()%256;
		write(out, &c, 1);
	}
}

int main(int argn, char **argv)
{
	char *name;
	int f_src, f_dst;

	if (!strcmp(argv[1], "help") && argn == 2)
		printf(HELP_MSG);
	else if (!strcmp(argv[1], "-f") && argn >= 3) {
		srand(time(NULL));
		for (int i = 2; i < argn; i++) {
			f_src = open(argv[i], O_RDONLY);
			if (f_src == -1) {
				perror("Error");
				continue;
			};
			// Adding .xored to filename
			name = (char *) malloc(strlen(argv[i]) + 7);
			HANDLE_ERROR(name, NULL);
			strcpy(name, argv[i]);
			strcat(name, ".xored");
			f_dst = creat(name, 0666);
			HANDLE_ERROR(f_dst, -1);
			generate(f_src, f_dst);
			free(name);
		}
	} else if (!strcmp(argv[1], "-k")
		&& argn == 3)
		printf("%s\n", argv[2]);
	else
		printf("Wrong usage. Try \"xorator help\".\n");



	return 0;
}