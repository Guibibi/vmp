#include "mt_log.c"
#include "mt_map_test.c"
#include "mt_memory_test.c"
#include "mt_vector_test.c"

int main(int argc, char* argv[])
{
    mt_log_set_level(MT_LOG_DEBUG);

    mt_memory_test_main();
    mt_vector_test_main();
    mt_map_test_main();

    return 0;
}
