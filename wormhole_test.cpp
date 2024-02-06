// Copyright 2023-2024 Jefferson Amstutz
// SPDX-License-Identifier: Apache-2.0

// mpi
#include <mpi.h>
// std
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "RMAWindow.hpp"

int main()
{
  // Init //

  MPI_Init(nullptr, nullptr);

  {
    int rank = -1;
    int size = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Hello from rank '%i' of '%i' ranks\n", rank, size);

    // Create Window //

    wormhole::RMAWindow<int> window;
    window.resize(rank == 0 ? size : 0);
    window.fill_buffer(0);

    // Sync so the fill on rank 0 doesn't race with other node put() calls //

    MPI_Barrier(MPI_COMM_WORLD);

    // Send my rank to rank 0 //

    window.put(0, &rank, 1, rank);

    // Get rank 0 size //

    int remoteRank = -1;
    window.get(0, &remoteRank, 1);
    window.fence();

    // Print outputs //

    if (rank == 0) {
      printf("values on rank 0:");
      const int *values = window.data();
      for (size_t i = 0; i < window.size(); i++)
        printf(" %i", values[i]);
      printf("\n");
      fflush(stdout);
    }

    if (rank == 1) {
      printf("value from rank 0 on rank 1: %i\n", remoteRank);
      fflush(stdout);
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }

  // Cleanup //

  MPI_Finalize();
}
