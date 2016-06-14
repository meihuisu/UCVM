/**
 * @file ucvm2mesh_mpi.c
 * @brief MPI implemention of the ucvm2mesh utility. Generates a Cartesian mesh from a velocity model.
 * @author David Gill - SCEC <davidgil@usc.edu>
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Generates a Cartesian mesh from one of the installed velocity models using MPI and MPI I/O.
 * This utility is used to generate meshes for various full 3D wave propagation simulation
 * programs such as AWP-ODC.
 *
 */

#include <mpi.h>


/**
 * Displays the usage information for this utility.
 *
 * @param arg The program's name. In this case it's ucvm2mesh.
 */
void usage(char *arg) {
	printf("Usage: %s [-h] -f configfile\n\n", arg);

	printf("where:\n");
	printf("\t-h: help message\n");
	printf("\t-f: configuration file containing mesh parameters\n\n");

	printf("Configuration file format:\n");
	printf("\tucvmlist: comma-delimited list of CVMs to query (as supported by UCVM)\n");
	printf("\tucvmconf: UCVM API config file\n");
	printf("\tgridtype: location of x-y gridded points: VERTEX, or CENTER\n");
	printf("\tspacing: grid spacing (units appropriate for proj)\n");
	printf("\tproj: Proj.4 projection specification, or 'cmu' for TeraShake\n");
	printf("\trot: proj rotation angle in degrees, (+ is counter-clockwise)\n");
	printf("\tx0: longitude of origin (deg), or x offset in cmu proj (m)\n");
	printf("\ty0: latitude of origin (deg), or y offset in cmu proj (m)\n");
	printf("\tz0: depth of origin (m, typically 0.0)\n");
	printf("\tnx: number of points along x-axis\n");
	printf("\tny: number of points along y-axis\n");
	printf("\tnz: number of points along z-axis (depth positive)\n");
	printf("\tpx: number of procs along x-axis\n");
	printf("\tpy: number of procs along y-axis\n");
	printf("\tpz: number of procs along z-axis\n");
	printf("\tvp_min: vp minimum (m/s), enforced on vs_min conditions\n");
	printf("\tvs_min: vs minimum (m/s)\n");
	printf("\tmeshfile: path and basename to output mesh files\n");
	printf("\tgridfile: path and filename to output grid files\n");
	printf("\tmeshtype: mesh format: IJK-12, IJK-20, IJK-32, or SORD\n");
	printf("\tscratch: path to scratch space\n\n");

	printf("Version: %s\n\n");

	return;
}

/**
 * Main routine for ucvm2mesh. Initializes the MPI routines, calls the configuration file
 * reading function, calls the extraction commands, and then cleans up.
 *
 * @param argc The number of arguments passed to the program.
 * @param argv The arguments passed. Minimally we need -f which specifies the file.
 * @return 0 on success, another code if an error occurs.
 */
int main(int argc, char **argv) {
	// Necessary MPI variables.
	int myid, nproc, pnlen;			// myid is MPI unique process id, nproc number of processors, pnlen?
	char procname[128];				// procname?

	// Initialize MPI.
	mpi_init(&argc, &argv, &nproc, &myid, procname, &pnlen);

	if (myid == 0) {
		// Let the user know we're starting.
		printf("%s utility version: %s\n", argv[0], VERSION);
		printf("Running on %d cores.\n", nproc);
		printf("Reading configuration file.\n", myid);
		fflush(stdout);
	}

	// Final MPI synchronization. Wait for all processes to reach here before terminating after.
	mpi_barrier();
	mpi_final("MPI Done");

	return 0;
}

/**
 * Prints the error string provided.
 *
 * @param err The error string to print out to stderr.
 */
void print_error(char *err) {
	fprintf(stderr, "An error has occurred while executing UCVM. The error was:\n\n");
	fprintf(stderr, "%s", err);
	fprintf(stderr, "\n\nPlease contact software@scec.org and describe both the error and a bit\n");
	fprintf(stderr, "about the computer on which you are running UCVM (Linux, Mac, etc.).\n");
}
