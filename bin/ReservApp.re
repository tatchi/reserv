Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Info));
Logs.set_reporter(Logs_fmt.reporter());

let reloadScript = "
<script>
    const source = new EventSource('http://localhost:5005');
    source.onmessage = e => location.reload(true);
</script>";

let handler = (request: Morph.Request.t(string)) => {
  open Morph;

  let path_parts =
    request.target
    |> Uri.of_string
    |> Uri.path
    |> String.split_on_char('/')
    |> List.filter(s => s != "");

  let get_file_stream = file_name => {
    let read_only_flags = [Unix.O_RDONLY];
    let read_only_perm = 444;
    let fd = Unix.openfile(file_name, read_only_flags, read_only_perm);
    Lwt_io.of_unix_fd(fd, ~mode=Lwt_io.Input) |> Lwt_io.read_chars;
  };

  switch (request.meth, path_parts) {
  | (`GET, file_path) =>
    let filePath = file_path |> String.concat("/");
    let file = Unix.stat(filePath);

    let sizeFile = file.st_size;

    let st = get_file_stream(filePath);
    let rl = Lwt_stream.of_string(reloadScript);

    let comb = Lwt_stream.append(st, rl);
    let size = sizeFile + String.length(reloadScript);

    Morph.Response.add_header(
      ("Content-type", Magic_mime.lookup(filePath)),
      Morph.Response.empty,
    )
    |> Response.add_header(("Transfer-Encoding", "chunked"))
    |> Morph.Response.add_header(("Content-length", string_of_int(size)))
    |> Morph_base.Response.byte_stream(~stream=comb);

  | (_, _) => Response.not_found(Response.empty)
  };
};

let http_server = Morph_server_http.make(~port=4000, ());

let reload_server = Morph_server_http.make(~port=5005, ());

let reloadServerHandler = (request: Morph.Request.t(string)) => {
  open Morph;

  // let stream =
  //   Lwt_process.pread_lines(("esy", [|"esy", "fswatch", "index.html"|]));

  let stream =
    Lwt_process.pread_lines((
      "esy",
      [|"esy", "fswatch", "-l 0.1", "-e '.*/\..*'", "."|],
    ));

  print_endline("new handler");

  let e = "event: message\nid: 0\ndata: change received\n\n\n";

  // let (st, push) = Lwt_stream.create();

  // let _ = push(Some("event: message\nid: 0\ndata: awaiting changes\n\n\n"));

  Response.empty
  |> Response.add_header(("Connection", "keep-alive"))
  |> Response.add_header(("Content-Type", "text/event-stream"))
  |> Response.add_header(("Cache-Control", "no-cache"))
  // |> Response.add_header(("Transfer-Encoding", "chunked"))
  |> Response.add_header(("Access-Control-Allow-Origin", "*"))
  |> Response.set_status(`OK)
  |> Morph_base.Response.string_stream(
       ~stream=stream |> Lwt_stream.map(_ => e),
     );
};

let main = () => {
  Lwt.join([
    Morph.start(
      ~servers=[http_server],
      ~middlewares=[Library.Middleware.logger],
      handler,
    ),
    Morph.start(
      ~servers=[reload_server],
      ~middlewares=[Library.Middleware.logger],
      reloadServerHandler,
    ),
  ]);
};

Lwt_main.run(main());