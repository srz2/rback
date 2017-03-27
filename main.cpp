// Steven Zilberberg
// March 24th, 2017
// Backup and import the linux routing table using the "route" command

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <vector>

#define RBACK_EXT ".rback"
#define FILE_NAME "archive" RBACK_EXT

typedef struct RouteData
{
	char dest[32];
	char gateway[32];
	char genmask[32];
	char flags[10];
	char metric[10];
	char ref[10];
	char use[10];
	char iface[10];

	char * output;

	RouteData()
	{
		output = NULL;
		memset(dest, '\0', sizeof(dest));
		memset(gateway, '\0', sizeof(gateway));
		memset(genmask, '\0', sizeof(genmask));
		memset(flags, '\0', sizeof(flags));
		memset(metric, '\0', sizeof(metric));
		memset(ref, '\0', sizeof(ref));
		memset(use, '\0', sizeof(use));
		memset(iface, '\0', sizeof(iface));
	}

	~RouteData()
	{
		delete [] output;
	}

	char * toString()
	{
		if(output == NULL)
			output = new char[1024];
		memset(output, '\0', 1024);

		sprintf(output, "%s,%s,%s,%s,%s,%s,%s,%s", this->dest, this->gateway, this->genmask, this->flags, this->metric, this->ref, this->use, this->iface);
		return output;
	}
}RouteData;

void showUsage();
void getCurrentWorkingDirectory(char * cwd, int size);
void parseArguments(int argc, char ** argv, char * dataSource, bool & isImporting, bool & isExporting);
void exportRouteTable(const char * dataSource);
void importRouteTable(const char * dataSource);

int main(int argc, char ** argv)
{
	char dataSource[1024];
	bool isImporting = false;
	bool isExporting = false;

	// Parse Arguments
	parseArguments(argc, argv, dataSource, isImporting, isExporting);
	if(isImporting && isExporting)
	{
		printf("[RBACK-ERR]: Choose to import or export route data. Not both.\n");
		exit(1);
	}

	uid_t uid = getuid();
	if(isImporting)
	{
		// Importing requires being root
		if(uid != 0)
		{
			printf("[RBACK-ERR]: Root user required for importing.\n");
			exit(1);
		}

		importRouteTable(dataSource);
	}
	else if(isExporting)
	{
		exportRouteTable(dataSource);
	}
	else
	{
		showUsage();
	}
}

void showUsage()
{
	printf("    Usage: rback [--export/--import] [-f export/import directory]\n");
	printf("        --export\n");
	printf("            export the route table\n");
	printf("        --import\n");
	printf("            import archived route information (this will not replace the data)\n");
	printf("        -f\n");
	printf("            directory from where export/import will be performed\n");
}

void getCurrentWorkingDirectory(char * cwd, int size)
{
	if(getcwd(cwd, size) == NULL)
		printf("[RBACK-ERR]: Unable to get working directory\n");
}

void parseArguments(int argc, char ** argv, char * dataSource, bool & isImporting, bool & isExporting)
{
	bool getCWD = true;

	// Evaluate arguments
	for(int c = 1; c < argc; c++)
	{
		if(strcmp(argv[c], "--import") == 0)
		{
			isImporting = true;
		}
		else if(strcmp(argv[c], "--export") == 0)
		{
			isExporting = true;
		}
		else if(strcmp(argv[c], "-f") == 0)
		{
			// Increment counter to absorb value
			c++;
			getCWD = false;

			// Copy path
			// Copy path to internal
			memset(dataSource, '\0', (int)(sizeof dataSource));
			strcpy(dataSource, argv[c]);

			// Check for extension
			if(strstr(dataSource, RBACK_EXT) == NULL)
				strcat(dataSource, RBACK_EXT);
		}
		else
		{
			printf("[RBACK-ERR]: Invalid command \'%s\'\n", argv[c]);
			exit(1);
		}
	}

	if(getCWD)
	{
		// Get directory from where program was called
		char dir[1024];
		memset(dir, '\0', sizeof(dir));
		getCurrentWorkingDirectory(dir, sizeof(dir));

		// Copy path to internal
		memset(dataSource, '\0', (int)(sizeof dataSource));
		strcpy(dataSource, dir);
		strcat(dataSource, "/");
		strcat(dataSource, FILE_NAME);
	}
}

void exportRouteTable(const char * dataSource)
{
	char routeDataLine[1024];
	memset(routeDataLine, '\0', sizeof(routeDataLine));

	FILE * fd = popen("route -n" , "r");
	if(fd == NULL)
	{
		printf("[RBACK-ERR]: Unable to retreive route table data\n");
		exit(1);
	}

	// Each loop iteration is a line
	int routeCount = 0;
	std::vector<RouteData> routes;
	while(fgets(routeDataLine, sizeof(routeDataLine), fd))
	{
		routeCount++;

		// Skip the first two lines
		if(routeCount <= 2)
			continue;

		// Delete the last two chars which is a newline and return
		routeDataLine[strlen(routeDataLine)] = 0x0;
		routeDataLine[strlen(routeDataLine) - 1] = 0x0;

		int paramCount = 0;
		RouteData newRoute;
		char * token = strtok(routeDataLine, " ");
		while(token != NULL)
		{
			switch(paramCount)
			{
				case 0:
					memcpy(newRoute.dest, token, strlen(token));
					break;
				case 1:
					memcpy(newRoute.gateway, token, strlen(token));
					break;
				case 2:
					memcpy(newRoute.genmask, token, strlen(token));
					break;
				case 3:
					memcpy(newRoute.flags, token, strlen(token));
					break;
				case 4:
					memcpy(newRoute.metric, token, strlen(token));
					break;
				case 5:
					memcpy(newRoute.ref, token, strlen(token));
					break;
				case 6:
					memcpy(newRoute.use, token, strlen(token));
					break;
				case 7:
					memcpy(newRoute.iface, token, strlen(token));
					break;
				default:
					printf("Unknown Parameter Count\n");
					break;
			}
			paramCount++;
			token = strtok(NULL, " ");
		}
		routes.push_back(newRoute);
	}
	pclose(fd);

	// Open file for output
	fd = fopen(dataSource, "w");
	if(fd == NULL)
	{
		printf("[RBACK-ERR]: Unable to write output to file \'%s\'\n", dataSource);
		exit(1);
	}

	// Output to file
	for(std::vector<RouteData>::iterator it = routes.begin(); it != routes.end(); it++)
	{
		RouteData data = *it;
		fprintf(fd, "%s\n", data.toString());
	}
	fclose(fd);

	printf("rBack has exported routing table to \'%s\'\n", dataSource);
}

void importRouteTable(const char * dataSource)
{
	FILE * fd = fopen(dataSource, "w");
	if(fd == NULL)
	{
		printf("[RBACK-ERR]: Unable to find archive at \'%s\'\n", dataSource);
		exit(1);
	}

	char line[1024];
	while(fgets(line, sizeof(line), fd))
	{
		printf("IMPORTED: %s\n", line);
	}

	fclose(fd);
}
