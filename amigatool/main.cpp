#include <amigadrive.h>
#include <amigadumpfile.h>
#include <amigaui.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;
using namespace amigadrive;

void showUsage(ConsoleUI *C)
{
	C->textWarning("\n");
	C->textWarning("Usage:\n");
	C->textWarning("    amigatool -f <dump file>\n");
	C->textWarning("        run amigatool for the given dump file\n");
	C->textWarning("\n");
	C->textWarning("    amigatool -d\n");
	C->textWarning("        describe the given dump file (-o)\n");
	C->textWarning("\n");
	C->textWarning("    amigatool -o <output file>\n");
	C->textWarning("        copy dump file to output file\n");
	C->textWarning("\n");
	C->textWarning("    amigatool -i <input file>\n");
	C->textWarning("        copy input file to dump file\n");
	C->textWarning("\n");
	C->textWarning("    amigatool -p <partition>\n");
	C->textWarning("        copy dump file partition number (use -d to view)\n");
	C->textWarning("\n");
	C->textWarning("    amigatool -b <start block>\n");
	C->textWarning("        copy dump file starting with this block\n");
	C->textWarning("\n");
	C->textWarning("    amigatool -s <count>\n");
	C->textWarning("        copy dump file for <size> blocks\n");
	C->textWarning("\n");
	C->textWarning("    amigatool -h <dump file>\n");
	C->textWarning("        output this text and exit.\n");
	C->textWarning("\n");
}

bool ifDescribe = false;

int main(int argc, char **argv)
{
	char *devname = nullptr;
	char *output = nullptr;
	char *input = nullptr;
    stringStore S;
	ConsoleUI C;	// All error, warning and info messages via console
	Device *D;		// Device
	ADFIO *A;		// Dump file IO driver
	long begin=-1;
	long size=-1;
	int partition=-1;
	int c;

	opterr = 0;

	while ((c = getopt (argc, argv, "p:b:s:di:o:f:h")) != -1)
		switch (c)
		{
			case 'p':
				partition = strtol(optarg, nullptr, 10);
				break;
			case 'b':
				begin = strtol(optarg, nullptr, 10);
				break;
			case 's':
				size = strtol(optarg, nullptr, 10);
				break;
			case 'd':
				ifDescribe = true;
				break;
			case 'o':
				output = S.copyString(optarg, strlen(optarg)+1);
				break;
			case 'i':
				input = S.copyString(optarg, strlen(optarg)+1);
				break;
			case 'f':
				devname = S.copyString(optarg, strlen(optarg)+1);
				break;
			case 'h':
				showUsage(&C);
				return 1;
				break;
			default:
				showUsage(&C);
				return 1;
				break;
		}

	if (!devname)
	{
		showUsage(&C);
		return 1;
	}

	try
	{
		A = new ADFIO();
		D = new Device(A, &C, devname, (output));

		if (ifDescribe && D)
        {
			D->About();
            return 0;
        }

        if (input && output)
        {
            C.textInfo("You may copy an input file into the dump file, or the dump file to an output file, but not both.\n");
        }

		if (partition > -1 && (begin > -1 || size > -1))
		{
			C.textInfo("The partition option is mutually exclusive with respect to the specification of the block indices (-b and -e).\n");
			showUsage(&C);
			return 1;
		}

		if (partition > D->volumeCount() || partition < 1)
		{
			C.textInfo("Check the partition numbers using the -d option.\n");
			showUsage(&C);
			return 1;
		}
		else
        {
            Volume *V = D->volumeNumber(partition);

            begin = V->volStartBlock();
            size = V->volBlockCount();
        }

        // C.textInfo("devname [%s], output [%s]\n", devname, output);

		if (devname && output && begin >-1 && size > -1)
		{
			C.textInfo("Copy dump file section [%s] to output [%s] from block %ld for %ld blocks\n\n", devname, output, begin, size);
			D->blockCopyOut(output, begin, size);
			C.textInfo("\n\nCopy complete.\n\n");
		}
		else
        {
            if (devname && input && begin >-1 && size > -1)
            {
                C.textInfo("Copy input file [%s] to dump file section [%s] from block %ld for %ld blocks\n\n", input, devname, begin, size);
                D->blockCopyIn(input, begin, size);
                C.textInfo("\n\nCopy complete.\n\n");
            }
        }

		C.textInfo("The end...\n");
		delete D;
		delete A;
	}
	catch (Exception E)
	{
		E.textMsg();
	}

	return 0;
}
