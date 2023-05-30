#include <iostream>
#include <cmath>
#include <random>
#include <chrono>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int np, pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (argc < 2) {
        if (pid == 0)
            std::cout << "Error: debe proporcionar el número de muestras (N) como argumento." << std::endl;
        MPI_Finalize();
        return 1;
    }

    int N = std::stoi(argv[1]);

    // Semilla aleatoria basada en el rango del proceso
    std::mt19937 rng(pid);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    // Iniciar el temporizador
    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();

    int local_count = 0;
    for (int i = 0; i < N; ++i) {
        double x = dist(rng);
        double y = dist(rng);
        if (x * x + y * y < 1.0)
            ++local_count;
    }

    if (pid != 0) {
        // Enviar el conteo local al proceso 0
        MPI_Send(&local_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else {
        int global_count = local_count;

        // Recibir los conteos locales de los otros procesos y sumarlos
        for (int i = 1; i < np; ++i) {
            int received_count;
            MPI_Recv(&received_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            global_count += received_count;
        }

        // Calcular el tiempo de ejecución
        std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);
        double elapsed_time = duration.count();

        double pi = 4.0 * static_cast<double>(global_count) / (N * np);
        double pi_diff = std::abs(pi - M_PI) / M_PI;

        std::cout.precision(15);
        std::cout << np << ", " << N << ", " << elapsed_time << ", " << pi_diff << std::endl;
    }

    MPI_Finalize();
    return 0;
}
