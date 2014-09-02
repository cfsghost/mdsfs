#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <time.h>

#include "genkey.h"

int main(int argc, char *argv[])
{
	gint len = 96;
	guint8 *key, *ptr;
	gchar table[16] = "0123456789abcdef";

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	/* allocate a memboy for 448 bits */
	key = (guint8 *)g_new0(guint8, len + 1);

	/* seed */
	srand(time(0)+getpid());
	for (ptr = key; len > 0; --len, ++ptr)
		*ptr = table[rand() % 16];

	printf("%s\n", key);

	g_free(key);

	return 0;
}
