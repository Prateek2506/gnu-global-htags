/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2002, 2008, 2011, 2014, 2015
 *	Tama Communications Corporation
 *
 * This file is part of GNU GLOBAL.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "gparam.h"
#include "abs2rel.h"
#include "die.h"
#include "getdbpath.h"
#include "gtagsop.h"
#include "locatestring.h"
#include "makepath.h"
#include "path.h"
#include "strlimcpy.h"
#include "test.h"

/**
 * define the position of the root slash.
 */
#if defined(_WIN32) || defined(__DJGPP__)
#define ROOT 2
#else
#define ROOT 0
#endif

static const char *gtagsobjdirprefix;	/**< tag partition	*/
static const char *gtagsobjdir;		/**< tag directory	*/
char const *gtags_dbpath_error;		/**< error message	*/

/**
 * setupvariables: load variables regard to BSD OBJ directory.
 */
static void
setupvariables(int verbose)
{
	const char *p;

	if ((p = getenv("GTAGSOBJDIRPREFIX")) != NULL ||
	    (p = getenv("MAKEOBJDIRPREFIX")) != NULL)
	{
		gtagsobjdirprefix = p;
		if (verbose)
			fprintf(stderr, "GTAGSOBJDIRPREFIX is set to '%s'.\n", p);
	} else {
		gtagsobjdirprefix = "/usr/obj";
	}
	if ((p = getenv("GTAGSOBJDIR")) != NULL ||
	    (p = getenv("MAKEOBJDIR")) != NULL)
	{
		gtagsobjdir = p;
		if (verbose)
			fprintf(stderr, "GTAGSOBJDIR is set to '%s'.\n", p);
	} else {
		gtagsobjdir = "obj";
	}
}
/**
 * getobjdir: get objdir if it exists.
 *
 *	@param[in]	candidate candidate root directory
 *	@param[in]	verbose	verbose mode 1: on, 0: off
 *	@return		objdir(NULL: not found)
 */
char *
getobjdir(const char *candidate, int verbose)
{
	static char path[MAXPATHLEN];

	/*
	 * setup gtagsobjdir and gtagsobjdirprefix (only first time).
	 */
	if (gtagsobjdir == NULL)
		setupvariables(0);
	snprintf(path, sizeof(path), "%s/%s", candidate, gtagsobjdir);
	if (test("d", path)) {
		if (!test("drw", path))
			die("Found objdir '%s', but you don't have read/write permission for it.", path);
		if (verbose)
			fprintf(stderr, "Using objdir '%s'.\n", path);
		return path;
	}
#if !defined(_WIN32) && !defined(__DJGPP__)
	if (test("d", gtagsobjdirprefix)) {
		snprintf(path, sizeof(path), "%s%s", gtagsobjdirprefix, candidate);
		if (test("d", path)) {
			if (!test("drw", path))
				die("Found objdir '%s', but you don't have read/write permission for it.", path);
			if (verbose)
				fprintf(stderr, "Using objdir '%s'.\n", path);
			return path;
		}
		if (makedirectories(gtagsobjdirprefix, candidate + 1, verbose) < 0)
			die("Found the base for objdir '%s', but you cannot create new directory in it.", path);
		if (verbose)
			fprintf(stderr, "Using objdir '%s'.\n", path);
		return path;
	}
#endif
	return NULL;
}
/**
 * gtagsexist: test whether GTAGS's existence.
 *
 *	@param[in]	candidate candidate root directory
 *	@param[out]	dbpath	directory which "GTAGS" exist
 *	@param[in]	size	size of dbpath buffer
 *	@param[in]	verbose	verbose mode 1: on, 0: off
 *	@return		0: not found, 1: found
 *
 * Gtagsexist locate "GTAGS" file in "$candidate/", "$candidate/obj/" and
 * "/usr/obj/$candidate/" in this order by default.
 * This behavior is same with BSD make(1)'s one.
 */
int
gtagsexist(const char *candidate, char *dbpath, int size, int verbose)
{
	char path[MAXPATHLEN];
	const char *candidate_without_slash;

	/*
	 * setup gtagsobjdir and gtagsobjdirprefix (only first time).
	 */
	if (gtagsobjdir == NULL)
		setupvariables(verbose);

	if (strcmp(candidate, "/") == 0)
		candidate_without_slash = "";
	else
		candidate_without_slash = candidate;
	snprintf(path, sizeof(path), "%s/%s", candidate_without_slash, dbname(GTAGS));
	if (verbose)
		fprintf(stderr, "checking %s\n", path);
	if (test("fr", path)) {
		if (verbose)
			fprintf(stderr, "GTAGS found at '%s'.\n", path);
		snprintf(dbpath, size, "%s", candidate);
		return 1;
	}
	snprintf(path, sizeof(path),
		"%s/%s/%s", candidate_without_slash, gtagsobjdir, dbname(GTAGS));
	if (verbose)
		fprintf(stderr, "checking %s\n", path);
	if (test("fr", path)) {
		if (verbose)
			fprintf(stderr, "GTAGS found at '%s'.\n", path);
		snprintf(dbpath, size, "%s/%s", candidate_without_slash, gtagsobjdir);
		return 1;
	}
#if !defined(_WIN32) && !defined(__DJGPP__)
	snprintf(path, sizeof(path),
		"%s%s/%s", gtagsobjdirprefix, candidate_without_slash, dbname(GTAGS));
	if (verbose)
		fprintf(stderr, "checking %s\n", path);
	if (test("fr", path)) {
		if (verbose)
			fprintf(stderr, "GTAGS found at '%s'.\n", path);
		snprintf(dbpath, size, "%s%s", gtagsobjdirprefix, candidate_without_slash);
		return 1;
	}
#endif
	return 0;
}
static char dbpath[MAXPATHLEN];
static char root[MAXPATHLEN];
static char root_with_slash[MAXPATHLEN];
static char cwd[MAXPATHLEN];
static char relative_cwd_with_slash[MAXPATHLEN+2];
/*
 * setupdbpath: setup dbpath directory
 *
 *	@param[in]	verbose	verbose mode 1: on, 0: off
 *	@return	0: normal, 0<: error
 *	Globals used (output):
 *		cwd:	current directory,
 *		root:	root of source tree,
 *		dbpath: directory which "GTAGS" exist,
 *		gtags_dbpath_error: set if status (return value) < 0
 *
 */
int
setupdbpath(int verbose)
{
	struct stat sb;
	char *p;
	static char msg[1024];

	if (!vgetcwd(cwd, MAXPATHLEN)) {
		gtags_dbpath_error = "cannot get current directory.";
		return -1;
	}
	canonpath(cwd);

	if ((p = getenv("GTAGSROOT")) != NULL) {
		if (verbose)
			fprintf(stderr, "GTAGSROOT is set to '%s'.\n", p);
		if (!isabspath(p)) {
			gtags_dbpath_error = "GTAGSROOT must be an absolute path.";
			return -1;
		}
		if (stat(p, &sb) || !S_ISDIR(sb.st_mode)) {
			snprintf(msg, sizeof(msg), "directory '%s' not found.", p);
			gtags_dbpath_error = msg;
			return -1;
		}
		if (getenv("GTAGSLOGICALPATH")) {
			strlimcpy(root, p, sizeof(root));
		} else {
			if (realpath(p, root) == NULL) {
				snprintf(msg, sizeof(msg), "cannot get real path of '%s'.", p);
				gtags_dbpath_error = msg;
				return -1;
			}
		}
		/*
		 * GTAGSDBPATH is meaningful only when GTAGSROOT exist.
		 */
		if ((p = getenv("GTAGSDBPATH")) != NULL) {
			if (verbose)
				fprintf(stderr, "GTAGSDBPATH is set to '%s'.\n", p);
			if (!isabspath(p)) {
				gtags_dbpath_error = "GTAGSDBPATH must be an absolute path.";
				return -1;
			}
			if (stat(p, &sb) || !S_ISDIR(sb.st_mode)) {
				snprintf(msg, sizeof(msg), "directory '%s' not found.", p);
				gtags_dbpath_error = msg;
				return -1;
			}
			strlimcpy(dbpath, getenv("GTAGSDBPATH"), MAXPATHLEN);
		} else {
			if (!gtagsexist(root, dbpath, MAXPATHLEN, verbose)) {
				gtags_dbpath_error = "GTAGS not found.";
				return -3;
			}
		}
	} else {
		if (verbose && getenv("GTAGSDBPATH"))
			fprintf(stderr, "warning: GTAGSDBPATH is ignored because GTAGSROOT is not set.\n");
		/*
		 * start from current directory to '/' directory.
		 */
		strlimcpy(root, cwd, MAXPATHLEN);
		p = root + strlen(root);
		while (!gtagsexist(root, dbpath, MAXPATHLEN, verbose)) {
			if (!strcmp(root+ROOT, "/")) { 	/* reached the system's root directory */
				*(root+ROOT) = '\0';
				break;
			}
			while (*--p != '/' && p > (root+ROOT))
				;
			if (p == (root+ROOT))
				p++;
			*p = 0;
		}
		if (*(root+ROOT) == 0) {
			gtags_dbpath_error = "GTAGS not found.";
			return -3;
		}
		/*
		 * If file 'GTAGSROOT' found without environment variable
		 * GTAGSDBPATH, use the value of it as GTAGSROOT.
		 */
		do {
			FILE *fp;
			STRBUF *sb;
			const char *s, *path;

			path = makepath(root, "GTAGSROOT", NULL);
			if (!test("fr", path)) {
				break;
			}
			fp = fopen(path, "r");
			if (fp == NULL) {
				if (verbose)
					fprintf(stderr, "'%s' ignored because it cannot be opened.\n", path);
				break;
			}
			sb = strbuf_open(0);
			s = strbuf_fgets(sb, fp, STRBUF_NOCRLF);
			if (s == NULL) {
				/* Empty file? */
				if (verbose)
					fprintf(stderr, "'%s' ignored; strbuf_fgets() returned NULL.\n", path);
				goto ignore_lab;
			}
			if (!test("d", s)) {	/* XXX: if s == NULL, test() will use previous path used. */
				if (verbose)
					fprintf(stderr, "'%s' ignored because it doesn't include existent directory name.\n", path);
			} else {
				char buf[MAXPATHLEN];

				if (verbose)
					fprintf(stderr, "GTAGSROOT found at '%s'.\n", path);
				if (!isabspath(s))
					s = realpath(makepath(root, s, NULL), buf);
				strlimcpy(root, s, MAXPATHLEN);
			}
ignore_lab:;
			fclose(fp);
			strbuf_close(sb);
			break;
		} while (0);
	}
	if (!strcmp(root+ROOT, "/"))
		strlimcpy(root_with_slash, root, sizeof(root_with_slash));
	else
		snprintf(root_with_slash, sizeof(root_with_slash), "%s/", root);
	/*
	 * relative from the project root directory.
	 */
	snprintf(relative_cwd_with_slash,
		sizeof(relative_cwd_with_slash), "./%s/", cwd + strlen(root) + 1);
	return 0;
}
/**
 * in_the_project: test whether path is in the project.
 *
 *	@param[in]	path	target file or directory
 *	@return		0: out of the project, 1: in the project
 *
 * [Note] Please pass an absolute path name which does not include "." (current directory) or
 *       ".." (parent directory).
 */
int
in_the_project(const char *path)
{
	if (!strcmp(path, root) || locatestring(path, root_with_slash, MATCH_AT_FIRST))
		return 1;
	return 0;
}
/*
 * return saved values.
 */
const char *
get_dbpath(void)
{
	return (const char *)dbpath;
}
const char *
get_root(void)
{
	return (const char *)root;
}
const char *
get_root_with_slash(void)
{
	return (const char *)root_with_slash;
}
const char *
get_cwd(void)
{
	return (const char *)cwd;
}
const char *
get_relative_cwd_with_slash(void)
{
	return (const char *)relative_cwd_with_slash;
}
void
dump_dbpath(void)
{
	fprintf(stderr, "db path: %s\n", dbpath);
	fprintf(stderr, "root path: %s\n", root);
	fprintf(stderr, "current directory: %s\n", cwd);
}
