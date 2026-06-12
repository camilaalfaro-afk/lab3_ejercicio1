#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>

int mi_strlen(const char *cadena) {
    int contador = 0;

    while (cadena[contador] != '\0') {
        contador++;
    }

    return contador;
}

void escribir_texto(int fd, const char *texto) {
    syscall(SYS_write, fd, texto, mi_strlen(texto));
}

void escribir_numero(int fd, long numero) {
    char buffer[20];
    int i = 0;

    if (numero == 0) {
        syscall(SYS_write, fd, "0", 1);
        return;
    }

    while (numero > 0) {
        buffer[i] = (numero % 10) + '0';
        numero = numero / 10;
        i++;
    }

    for (int j = i - 1; j >= 0; j--) {
        syscall(SYS_write, fd, &buffer[j], 1);
    }
}

void escribir_dos_digitos(int fd, int numero) {
    if (numero < 10) {
        syscall(SYS_write, fd, "0", 1);
    }

    escribir_numero(fd, numero);
}

int es_bisiesto(int anio) {
    if (anio % 400 == 0) {
        return 1;
    }

    if (anio % 100 == 0) {
        return 0;
    }

    if (anio % 4 == 0) {
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        escribir_texto(2, "Error: debe proporcionar el nombre de usuario.\n");
        escribir_texto(2, "Uso: ./ejercicio1 <usuario>\n");
        return 1;
    }

    long tiempo = syscall(SYS_time, 0);
    tiempo = tiempo - (5 * 3600);

    long dias = tiempo / 86400;
    long segundos_hoy = tiempo % 86400;

    int hora = segundos_hoy / 3600;
    int min = (segundos_hoy % 3600) / 60;
    int seg = segundos_hoy % 60;

    int anio = 1970;

    while (dias >= 365 + es_bisiesto(anio)) {
        dias = dias - (365 + es_bisiesto(anio));
        anio++;
    }

    int dias_por_mes[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (es_bisiesto(anio)) {
        dias_por_mes[1] = 29;
    }

    int mes = 1;

    while (dias >= dias_por_mes[mes - 1]) {
        dias = dias - dias_por_mes[mes - 1];
        mes++;
    }

    int dia = dias + 1;

    int fd = syscall(
        SYS_open,
        "access_log.txt",
        O_WRONLY | O_CREAT | O_APPEND,
        0644
    );

    if (fd < 0) {
        escribir_texto(2, "Error: no se pudo abrir access_log.txt\n");
        return 1;
    }

    long pid = syscall(SYS_getpid);

    syscall(SYS_write, fd, "[", 1);

    escribir_numero(fd, anio);
    syscall(SYS_write, fd, "-", 1);

    escribir_dos_digitos(fd, mes);
    syscall(SYS_write, fd, "-", 1);

    escribir_dos_digitos(fd, dia);
    syscall(SYS_write, fd, " ", 1);

    escribir_dos_digitos(fd, hora);
    syscall(SYS_write, fd, ":", 1);

    escribir_dos_digitos(fd, min);
    syscall(SYS_write, fd, ":", 1);

    escribir_dos_digitos(fd, seg);

    escribir_texto(fd, "] Usuario: ");
    escribir_texto(fd, argv[1]);
    escribir_texto(fd, " (PID=");
    escribir_numero(fd, pid);
    escribir_texto(fd, ")\n");

    syscall(SYS_close, fd);

    escribir_texto(1, "Registro actualizado en access_log.txt\n");

    return 0;
}
