let () = {
  Console.log("Hello, world!");
  let watcher = Luv.FS_event.init() |> Result.get_ok;

  Luv.FS_event.start(
    watcher,
    "index.html",
    fun
    | Error(e) => {
        Console.log("error");
      }
    | Ok((file, events)) => {
        if (List.mem(`RENAME, events)) {
          Console.log("rename");
        };
        if (List.mem(`CHANGE, events)) {
          Console.log("change" ++ file);
        };
      },
  );
};
ignore(Luv.Loop.run());