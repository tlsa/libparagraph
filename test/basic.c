#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <dom/dom.h>
#include <dom/walk.h>

#include <libcss/libcss.h>

#include <paragraph.h>

#include <sd/sd.h>

static bool run_test(
		const char *css,
		const char *html)
{
	dom_document *doc;
	bool res;

	res = paragraph_sd_ctx_init();
	if (res != true) {
		return res;
	}

	res = sd_load_data(css, html, &doc);
	if (res != true) {
		paragraph_sd_ctx_fini();
		return res;
	}

	sd_free(doc);

	paragraph_sd_ctx_fini();
	return true;
}

static bool example_latin(void)
{
	const char *html =
			"<html>\n"
			"  <head><title>Title</title></head>\n"
			"  <body>\n"
			"    <p>This is a <em>simple</em> test!\n"
			"  </body>\n"
			"</html>\n";
	const char *css =
			"p > em {font_size: 200%;}\n";

	return run_test(css, html);
}

int main(int argc, char *argv[])
{
	bool res;

	UNUSED(argc);
	UNUSED(argv);

	if (!sd_init(
			"html, body, p { display: block }"
			"head { display: none }",
			false)) {
		return EXIT_FAILURE;
	}

	res = example_latin();
	sd_fini();
	if (res != true) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
