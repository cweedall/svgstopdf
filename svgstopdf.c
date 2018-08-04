/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set sw=4 sts=4 ts=4 expandtab: */
/*
   svgstopdf.c: Command line utility for building multipage PDFs from multiple SVGs
 
   Copyright (C) 2016 Arnaud Lejosne <ad-gh@arnaud-lejosne.com>
   Copyright (C) 2018 Christopher Weedall <cw.coder@gmail.com>

   Several parts of the code were insipred by this Gist https://gist.github.com/julian-klode/ac45de6d4d2227d9febc
   as well as LibRSVG's rsvg-convert utility https://git.gnome.org/browse/librsvg/plain/rsvg-convert.c
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Authors: Arnaud Lejosne <ad-gh@arnaud-lejosne.com>
   Authors: Christopher Weedall <cw.coder@gmail.com>
*/

#define _GNU_SOURCE
#include <fontconfig.h>
#include <fcfreetype.h>
#include <freetype/ftcache.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <librsvg/rsvg.h>
#include <string.h>

static cairo_status_t
rsvg_cairo_write_func (void *closure, const unsigned char *data, unsigned int length)
{
    if (fwrite (data, 1, length, (FILE *) closure) == length)
        return CAIRO_STATUS_SUCCESS;
    return CAIRO_STATUS_WRITE_ERROR;
}

int
main(int argc, char *argv[])
{
    GOptionContext *g_option_context;
    char **args = NULL;
    gint n_args = 0;
    cairo_surface_t *surface = NULL;
    cairo_t *cr = NULL;
    GError *error = NULL;
    RsvgHandle *rsvg = NULL;
    RsvgHandleFlags flags = RSVG_HANDLE_FLAGS_NONE;
    RsvgDimensionData dimensions;
    gboolean create_latex_package = FALSE;
    FILE *tex_file = NULL;

    GOptionEntry options_table[] = {
        {"latex-package", 0, 0, G_OPTION_ARG_NONE, &create_latex_package, "Create LaTeX package", NULL},
        {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &args, NULL, "[1.svg 2.svg ... out.pdf]"},
        {NULL}
    };

    g_option_context = g_option_context_new ("- SVGs to PDF");
    g_option_context_add_main_entries(g_option_context, options_table, NULL);
    g_option_context_set_help_enabled(g_option_context, TRUE);

    if (!g_option_context_parse(g_option_context, &argc, &argv, &error)) {
        g_option_context_free(g_option_context);
        g_printerr("%s\n", error->message);
        exit(1);
    }

    g_option_context_free(g_option_context);

    if (args) {
        while (args[n_args] != NULL) {
            n_args++;
        }
    }
    if (n_args < 2) {
        g_printerr("Usage: svgstopdf 1.svg 2.svg ... out.pdf\n");
		g_printerr("Usage: svgstopdf *.svg ... out.pdf\n");
        exit(1);
    }

    char* output = args[n_args - 1];
    int outlen = strlen(output);
    char* point;

    if((point = strrchr(output,'.')) != NULL) {
        if(strcmp(point,".pdf") != 0) {
            g_printerr("The output file should be a .pdf file\n");
            exit(1);
        }
    }


    FILE *output_file = fopen(output, "wb");

    if(!output_file) {
        g_printerr("Error opening output file: %s\n", args[n_args - 1]);
        exit(1);
    }
	/*
		Create LaTeX output package - svgimages.sty
		
		This package file should be included: \usepackage{svgimages}
		from the same directory as the .tex file that you are building
	*/
    if(create_latex_package) {
        tex_file = fopen("svgimages.sty", "wb");
		/*
			Cannot create the .sty file.  Maybe it's already open and locked by another program?
		*/
        if(!tex_file) {
            g_printerr("Error opening LaTeX output file: %s\n", args[n_args - 1]);
            exit(1);
        }
		fputs("%%% Name of the package: svgimages\n", tex_file);
		fputs("\\ProvidesPackage{svgimages}%\n", tex_file);
		fputs("%%% Require xifthen package for if/then logic\n", tex_file);
        fputs("\\RequirePackage{xifthen}%\n", tex_file);
		fputs("%%% Require keyval package for accepting options passed to the package\n", tex_file);
		fputs("%%% This is useful for the user to define a PATH to the PDF file,\n", tex_file);
		fputs("%%% if you keep it (for example) in a subfolder -- i.e. not the same directory as the .tex file\n", tex_file);
        fputs("\\RequirePackage{keyval}%\n", tex_file);
		fputs("%%% Require graphicx package, for loading PDF files\n", tex_file);
		fputs("\\RequirePackage{graphicx}%\n", tex_file);
		fputs("%%% Package uses @ symbol.  Therefore, make it a regular character (i.e. not special, reserved character)\n", tex_file);
		fputs("\\makeatletter%\n", tex_file);
		fputs("%%% PATH option for package -- it defines the path to the PDF file\n", tex_file);
		fputs("\\DeclareOptionX{path}{\\def\\Ginput@path@svgimages{#1}}%\n", tex_file);
		fputs("%%% If an unknown option is passed to package, display a warning and details\n", tex_file);
		fputs("\\DeclareOptionX*{\\PackageWarning{svgimages}{`\\CurrentOption' ignored}}% For unknown options\n", tex_file);
		fputs("%%% Specify the default value for PATH (i.e. same folder as the .tex file), if it is not specified\n", tex_file);
		fputs("\\ExecuteOptionsX{path={./}}% Preset keys, 'section' being the default here\n", tex_file);
		fputs("%%% Process all package option setup before proceeding\n", tex_file);
		fputs("\\ProcessOptionsX\\relax%\n", tex_file);
		fputs("%%% If-then-else else logic.  This is defined specially so that it does not interfere with other packages.\n", tex_file);
        fputs("\\newcommand{\\@if@equals@svgimages}[3]{\\ifthenelse{\\equal{#1}{#2}}{#3}{}}%\n", tex_file);
		fputs("%%% Case logic -- i.e. if the value of the switch command is XYZ, then print this \\includesvg{} command.\n", tex_file);
        fputs("\\newcommand{\\@case@svgimages}[2]{#1 #2}%\n", tex_file);
		fputs("%%% Define Switch environment for determining the current value of \\includesvg{} command \n", tex_file);
        fputs("\\newenvironment{switch@svgimages}[1]{\\renewcommand{\\@case@svgimages}{\\@if@equals@svgimages{#1}}}{}%\n", tex_file);
		fputs("%%% \\includesvg{} command performs automagically and specifies to the \\includegraphics{} command which page to display\n", tex_file);
        fputs("\\newcommand{\\includesvg}[2][]{%\n", tex_file);
		fputs("%%% Start Switch environment\n", tex_file);
        fputs("\t\\begin{switch@svgimages}{#2}%\n", tex_file);
    }

    /*
     * We initialize the surface with DIN A4 default
     */
    surface = cairo_pdf_surface_create_for_stream(rsvg_cairo_write_func, output_file, 595.276, 841.89);
    cr = cairo_create(surface);

    int i;

    for (i = 0; i < n_args - 1; i++) {
        GFile *file;
        GInputStream *stream;

        file = g_file_new_for_commandline_arg(args[i]);
        stream = (GInputStream *) g_file_read(file, NULL, &error);

        rsvg = rsvg_handle_new_from_stream_sync(stream, file, flags, NULL, &error);

        g_clear_object(&stream);
        g_clear_object(&file);

        if (error != NULL) {
            g_printerr("Error reading SVG:");
            g_printerr("%s\n", error->message);
            g_printerr("\n");
            exit(1);
        }
        if (!rsvg_handle_get_dimensions_sub(rsvg, &dimensions, NULL)) {
            g_printerr("Could not get dimensions for file %s\n", args[i]);
            g_printerr("\n");
            exit(1);
        }

        cairo_pdf_surface_set_size(surface, dimensions.width, dimensions.height);

        rsvg_handle_render_cairo_sub(rsvg, cr, NULL);
        cairo_show_page(cr);
        g_object_unref(rsvg);

        if (create_latex_package) {
            char* string;
			asprintf(&string, "\t\t\\@case@svgimages{%s}{\\begingroup\\graphicspath{\\Ginput@path@svgimages}\\includegraphics[#1,page=%d]{%s}\\endgroup}%%\n", args[i], i + 1, output);
            fputs(string, tex_file);
            free(string);
        }
    }

    if (create_latex_package) {
		fputs("%%% End Switch environment\n", tex_file);
		fputs("\t\\end{switch@svgimages}%\n", tex_file);
		fputs("%%% End of \\includesvg{} command\n", tex_file);
        fputs("}%\n", tex_file);
		fputs("%%% Return @ character to its typical, special, reserved status\n", tex_file);
		fputs("\\makeatother%", tex_file);
        fclose(tex_file);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    fclose(output_file);
    return 0;
}
