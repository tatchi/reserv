let reloadScript = "
<script>
    const source = new EventSource('/livereload');
    const reload = ()=> {
      console.log('received')
      location.reload(true)
      };
    source.onmessage = reload;
    source.onerror = () =>{
      console.log('error')
      source.onopen = reload
      };
    console.log('[reserv] listening for file changes');
</script>";

let (stream, push) = Lwt_stream.create();

let getFiles = (dir: string) => {
  let rec helper = (dirString: string) => {
    switch (Luv.File.Sync.opendir(dirString)) {
    | Error(_) =>
      Console.log("error while opening dir");
      [||];
    | Ok(dir) =>
      switch (Luv.File.Sync.readdir(dir)) {
      | Error(_) =>
        Console.log("error while reading dir");
        [||];
      | Ok(dirents) =>
        Array.fold_left(
          (acc, dirent) =>
            switch (Luv.File.Dirent.(dirent.kind)) {
            | `FILE =>
              Array.append(
                [|dirString ++ "/" ++ Luv.File.Dirent.(dirent.name)|],
                acc,
              )
            | `DIR =>
              Array.append(
                helper(dirString ++ "/" ++ Luv.File.Dirent.(dirent.name)),
                acc,
              )
            | _ => acc
            },
          [||],
          dirents,
        )
      }
    };
  };

  helper(dir);
};

getFiles("build")
|> Array.iter(filename => {
     Console.log(filename);
     let watcher = Luv.FS_poll.init() |> Result.get_ok;
     let watch =
       Luv.FS_poll.start(~interval=10, watcher, filename, result => {
         switch (result) {
         | Error(e) => Console.log(e)
         | Ok((fileStat1, fileStat2)) =>
           Console.log("change");
           push(Some("event: message\nid: 0\ndata: reload\n \n\n"));
         }
       });
     let threadFs = Luv.Thread.create(_ => watch) |> Result.get_ok;
     ignore(Luv.Thread.join(threadFs));
   });

let isBaseRoute = (request: Morph.Request.t(string)) =>
  Uri.of_string(request.target) |> Uri.path == "/index.html";

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
  | (`GET, ["livereload"]) =>
    open Morph;
    push(Some("event: connected\nid: 0\ndata: ready\n \n\n"));
    Response.empty
    |> Response.add_header(("Connection", "keep-alive"))
    |> Response.add_header(("Content-Type", "text/event-stream"))
    |> Response.add_header(("Cache-Control", "no-cache"))
    |> Response.add_header(("Transfer-Encoding", "chunked"))
    |> Response.add_header(("Access-Control-Allow-Origin", "*"))
    |> Response.set_status(`OK)
    |> Morph_base.Response.string_stream(~stream);
  | (`GET, file_path) =>
    let filePath = ["build", ...file_path] |> String.concat("/");

    switch (Luv.File.Sync.stat(filePath)) {
    | Error(_) => Response.not_found(Response.empty)
    | Ok(fileStat) =>
      let sizeFile = Unsigned.UInt64.to_int(fileStat.size);
      let streamFile = get_file_stream(filePath);

      let (stream, size) =
        switch (isBaseRoute(request)) {
        | false => (streamFile, sizeFile)
        | true =>
          let reloadScriptStream = Lwt_stream.of_string(reloadScript);
          let combinedStream =
            Lwt_stream.append(streamFile, reloadScriptStream);
          let combinedSize = sizeFile + String.length(reloadScript);

          (combinedStream, combinedSize);
        };

      Morph.Response.add_header(
        ("Content-type", Magic_mime.lookup(filePath)),
        Morph.Response.empty,
      )
      |> Response.add_header(("Transfer-Encoding", "chunked"))
      |> Morph.Response.add_header(("Content-length", string_of_int(size)))
      |> Morph_base.Response.byte_stream(~stream);
    };

  | (_, _) => Response.not_found(Response.empty)
  };
};

let run = () => {
  Fmt_tty.setup_std_outputs();
  Logs.set_level(Some(Logs.Info));
  Logs.set_reporter(Logs_fmt.reporter());
  let startServer = () =>
    Morph.start(
      ~servers=[Morph_server_http.make(~port=4000, ())],
      ~middlewares=[Library.Middleware.logger],
      handler,
    );
  ignore(
    Luv.Thread.create(_ => Lwt_main.run(startServer())) |> Result.get_ok,
  );
  Luv.Loop.run();
};

run();