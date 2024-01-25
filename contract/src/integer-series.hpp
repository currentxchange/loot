    // --- Series Constants --- //
    //Also accepts ZERO which sets rewards to Zero and COUNT which counts from one. 
    namespace series {
    
    const std::vector<uint64_t> FIBONACCI = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 9999999999999999999};
    const std::vector<uint64_t> SILVER = {1, 2, 5, 12, 29, 70, 169, 408, 985, 2378, 5741, 13860, 33461, 80782, 195025, 470832, 1136689, 2744210, 6625109, 15994428, 38613965, 93222358, 225058681, 54333972, 9999999999999999999};

    const std::vector<uint64_t> TETRAHEDRAL = {1, 4, 10, 20, 35, 56, 84, 120, 165, 220, 286, 364, 455, 560, 680, 816, 969, 1140, 1330, 1540, 1771, 2024, 2300, 2600, 9999999999999999999};
    const std::vector<uint64_t> OCTAHEDRAL = {1, 6, 19, 44, 85, 146, 231, 344, 489, 670, 891, 1156, 1471, 1842, 2275, 2776, 3351, 4006, 4747, 5580, 6511, 7546, 8691, 9952, 9999999999999999999};
    const std::vector<uint64_t> HEXAHEDRAL = {1, 8, 27, 64, 125, 216, 343, 512, 729, 1000, 1331, 1728, 2197, 2744, 3375, 4096, 4913, 5832, 6859, 8000, 9261, 10648, 12167, 13824, 9999999999999999999};
    const std::vector<uint64_t> ICOSAHEDRAL = {1, 12, 42, 92, 162, 252, 362, 492, 642, 812, 1002, 1212, 1442, 1692, 1962, 2252, 2562, 2892, 3242, 3612, 4002, 4412, 4842, 5292, 9999999999999999999};
    const std::vector<uint64_t> DODECAHEDRAL = {1, 20, 84, 220, 455, 812, 1330, 2024, 2925, 4060, 5456, 7140, 9141, 11480, 14156, 17264, 20899, 25056, 29830, 35216, 41219, 47844, 55196, 63280, 9999999999999999999};

    const std::vector<uint64_t> LUCAS = {2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123, 199, 322, 521, 843, 1364, 2207, 3571, 5778, 9349, 15127, 24476, 39603, 64079, 9999999999999999999};
    const std::vector<uint64_t> TRIANGULAR = {1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171, 190, 210, 231, 253, 276, 300, 9999999999999999999};
    const std::vector<uint64_t> SQUARE = {1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 169, 196, 225, 256, 289, 324, 361, 400, 441, 484, 529, 576, 9999999999999999999};
    const std::vector<uint64_t> PENTAGONAL = {1, 5, 12, 22, 35, 51, 70, 92, 117, 145, 176, 210, 247, 287, 330, 376, 425, 477, 532, 590, 651, 715, 782, 852, 9999999999999999999};
    const std::vector<uint64_t> HEXAGONAL = {1, 6, 15, 28, 45, 66, 91, 120, 153, 190, 231, 276, 325, 378, 435, 496, 561, 630, 703, 780, 861, 946, 1035, 1128, 9999999999999999999};

    }


        // --- Return a Series --- //
    vector<uint64_t> getSeries(const string& seriesName){
        if (seriesName == "FIBONACCI") {
            return series::FIBONACCI;
        } else if (seriesName == "SILVER") {
            return series::SILVER;
        } else if (seriesName == "TETRAHEDRAL") {
            return series::TETRAHEDRAL;
        } else if (seriesName == "OCTAHEDRAL") {
            return series::OCTAHEDRAL;
        } else if (seriesName == "HEXAHEDRAL") {
            return series::HEXAHEDRAL;
        } else if (seriesName == "ICOSAHEDRAL") {
            return series::ICOSAHEDRAL;
        } else if (seriesName == "DODECAHEDRAL") {
            return series::DODECAHEDRAL;
        } else if (seriesName == "LUCAS") {
            return series::LUCAS;
        } else if (seriesName == "TRIANGULAR") {
            return series::TRIANGULAR;
        } else if (seriesName == "SQUARE") {
            return series::SQUARE;
        } else if (seriesName == "PENTAGONAL") {
            return series::PENTAGONAL;
        } else if (seriesName == "HEXAGONAL") {
            return series::HEXAGONAL;
        } else {
            return std::vector<uint64_t>();
        }
            
    }//END getSeries()

