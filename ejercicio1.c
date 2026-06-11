#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <fcntl.h>

static long my_strlen(const char *s) {
    long n = 0;
    while (s[n]) n++;
    return n;
}

static void write_str(int fd, const char *s) {
    syscall(SYS_write, fd, s, my_strlen(s));
}

static char *uint_to_str(unsigned long val, char *buf, int width, int zero_pad) {
    buf[width] = '\0';
    int i = width - 1;
    if (val == 0) {
        buf[i--] = '0';
    } else {
        while (val > 0 && i >= 0) {
            buf[i--] = '0' + (val % 10);
            val /= 10;
        }
    }
    char fill = zero_pad ? '0' : ' ';
    while (i >= 0) buf[i--] = fill;
    return buf;
}

static int is_leap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static int days_in_month(int m, int y) {
    int dm[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && is_leap(y)) return 29;
    return dm[m - 1];
}

static void epoch_to_datetime(long epoch,
                               int *year, int *month, int *day,
                               int *hour, int *min,  int *sec) {
    long day_secs = epoch % 86400;
    long days     = epoch / 86400;

    *sec  = (int)(day_secs % 60);
    *min  = (int)((day_secs / 60) % 60);
    *hour = (int)(day_secs / 3600);

    int y = 1970;
    while (1) {
        long days_in_year = is_leap(y) ? 366 : 365;
        if (days < days_in_year) break;
        days -= days_in_year;
        y++;
    }
    *year = y;

    int m = 1;
    while (1) {
        int dim = days_in_month(m, y);
        if (days < dim) break;
        days -= dim;
        m++;
    }
    *month = m;
    *day   = (int)days + 1;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        write_str(2, "Error: debe proporcionar el nombre de usuario.\n");
        write_str(2, "Uso: ./registro <nombre_usuario>\n");
        return 1;
    }

    /* 1. Obtener tiempo actual via SYS_time */
    long now = syscall(SYS_time, 0);

    /* 2. Convertir timestamp a fecha/hora */
    int year, month, day, hour, min, sec;
    epoch_to_datetime(now, &year, &month, &day, &hour, &min, &sec);

    /* 3. Obtener PID via SYS_getpid */
    long pid = syscall(SYS_getpid);

    /* 4. Construir la linea: [YYYY-MM-DD HH:MM:SS] Usuario: <nombre> (PID=XXXX)\n */
    char num_buf[12];
    char line[256];
    long pos = 0;

    line[pos++] = '[';

    uint_to_str((unsigned long)year, num_buf, 4, 1);
    for (int i = 0; i < 4; i++) line[pos++] = num_buf[i];
    line[pos++] = '-';

    uint_to_str((unsigned long)month, num_buf, 2, 1);
    line[pos++] = num_buf[0]; line[pos++] = num_buf[1];
    line[pos++] = '-';

    uint_to_str((unsigned long)day, num_buf, 2, 1);
    line[pos++] = num_buf[0]; line[pos++] = num_buf[1];
    line[pos++] = ' ';

    uint_to_str((unsigned long)hour, num_buf, 2, 1);
    line[pos++] = num_buf[0]; line[pos++] = num_buf[1];
    line[pos++] = ':';

    uint_to_str((unsigned long)min, num_buf, 2, 1);
    line[pos++] = num_buf[0]; line[pos++] = num_buf[1];
    line[pos++] = ':';

    uint_to_str((unsigned long)sec, num_buf, 2, 1);
    line[pos++] = num_buf[0]; line[pos++] = num_buf[1];

    const char *mid = "] Usuario: ";
    for (int i = 0; mid[i]; i++) line[pos++] = mid[i];

    for (int i = 0; argv[1][i]; i++) line[pos++] = argv[1][i];

    const char *pid_label = " (PID=";
    for (int i = 0; pid_label[i]; i++) line[pos++] = pid_label[i];

    uint_to_str((unsigned long)pid, num_buf, 10, 0);
    int start = 0;
    while (start < 9 && num_buf[start] == ' ') start++;
    for (int i = start; i < 10; i++) line[pos++] = num_buf[i];

    line[pos++] = ')';
    line[pos++] = '\n';

    /* 5. Abrir/crear el archivo con SYS_open */
    int fd = (int)syscall(SYS_open,
                          "access_log.txt",
                          O_WRONLY | O_CREAT | O_APPEND,
                          0644);
    if (fd < 0) {
        write_str(2, "Error: no se pudo abrir access_log.txt\n");
        return 1;
    }

    /* 6. Escribir con SYS_write */
    syscall(SYS_write, fd, line, pos);

    /* 7. Cerrar con SYS_close */
    syscall(SYS_close, fd);

    write_str(1, "Registro actualizado en access_log.txt\n");

    return 0;
}
