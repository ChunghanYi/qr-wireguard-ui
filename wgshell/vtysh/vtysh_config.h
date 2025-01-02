/*
 * Copyright (c) 2024 Chunghan Yi <chunghan.yi@gmail.com>
 */

#ifndef __VTYSH_CONFIG_H__
#define __VTYSH_CONFIG_H__

struct config {
	/* Configuration node name. */
	char *name;

	/* Configuration string line. */
	struct list *line;

	/* Index of this config. */
#if 0
	u_int32_t index;
#else
	unsigned int index;
#endif
};

struct config *config_get(int index, char *line);
void config_add_line(struct list *config, char *line, ...);
void config_del_line(struct list *config, char *line);
void config_del_line_byleft(struct list *config, char *line);
char *config_get_line_byleft(struct list *config, char *line);
void config_dump (FILE *fp);
void config_init ();

extern struct list *config_top;

#define ENSURE_CONFIG(v) do {	\
	if (v->type == VTY_FILE)		\
		return CMD_SUCCESS;		\
} while (0)

#endif
