unsigned int verbosity = 0;

void log_set_verbosity(unsigned int new_verbosity) {
    verbosity = new_verbosity;
}

unsigned int log_get_verbosity() {
    return verbosity;
}
